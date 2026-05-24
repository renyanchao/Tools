#include <cstring>
#include <iostream>
#include <string>

#include "Chat.pb.h"
#include "Login.pb.h"
#include "Packet.h"

#ifdef _WIN32
    #define _WIN32_WINNT 0x0601
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL   SOCKET_ERROR
    #define close_socket(s)    closesocket(s)
    #define GET_LAST_ERROR()   WSAGetLastError()
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL   (-1)
    #define close_socket(s)    close(s)
    #define GET_LAST_ERROR()   errno
#endif

const char* DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_PORT = 8888;

struct WirePacket
{
    PacketHeader header;
    std::string payload;
};

bool init_network()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
#endif
    return true;
}

void cleanup_network()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

socket_t connect_to_server(const std::string& ip, int port)
{
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL)
    {
        std::cerr << "socket() error: " << GET_LAST_ERROR() << std::endl;
        return INVALID_SOCKET_VAL;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0)
    {
        std::cerr << "inet_pton() error: invalid IP address" << std::endl;
        close_socket(sock);
        return INVALID_SOCKET_VAL;
    }

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR_VAL)
    {
        std::cerr << "connect() error: " << GET_LAST_ERROR() << std::endl;
        close_socket(sock);
        return INVALID_SOCKET_VAL;
    }

    return sock;
}

bool send_all(socket_t sock, const char* data, int total_len)
{
    int sent_total = 0;
    while (sent_total < total_len)
    {
        int sent = send(sock, data + sent_total, total_len - sent_total, 0);
        if (sent == SOCKET_ERROR_VAL || sent <= 0)
        {
            return false;
        }
        sent_total += sent;
    }
    return true;
}

bool recv_all(socket_t sock, char* data, int total_len)
{
    int recv_total = 0;
    while (recv_total < total_len)
    {
        int received = recv(sock, data + recv_total, total_len - recv_total, 0);
        if (received <= 0)
        {
            return false;
        }
        recv_total += received;
    }
    return true;
}

bool send_packet(socket_t sock, int32_t packet_id, const google::protobuf::Message& message)
{
    std::string payload;
    if (message.SerializeToString(&payload) == false)
    {
        return false;
    }

    PacketHeader header;
    header.m_nPacketId = packet_id;
    header.m_nPacketSize = static_cast<int32_t>(payload.size());

    if (send_all(sock, reinterpret_cast<const char*>(&header), static_cast<int>(sizeof(header))) == false)
    {
        return false;
    }

    if (payload.empty())
    {
        return true;
    }

    return send_all(sock, payload.data(), static_cast<int>(payload.size()));
}

bool recv_packet(socket_t sock, WirePacket& out_packet)
{
    if (recv_all(sock, reinterpret_cast<char*>(&out_packet.header), static_cast<int>(sizeof(out_packet.header))) == false)
    {
        return false;
    }

    if (out_packet.header.m_nPacketSize < 0)
    {
        return false;
    }

    out_packet.payload.resize(static_cast<size_t>(out_packet.header.m_nPacketSize));
    if (out_packet.header.m_nPacketSize == 0)
    {
        return true;
    }

    return recv_all(sock, &out_packet.payload[0], out_packet.header.m_nPacketSize);
}

int main(int argc, char* argv[])
{
    std::string server_ip = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    if (argc >= 2) server_ip = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    if (!init_network())
    {
        return 1;
    }

    socket_t sock = connect_to_server(server_ip, port);
    if (sock == INVALID_SOCKET_VAL)
    {
        cleanup_network();
        std::cout << "connect to server fail" << std::endl;
        return 1;
    }

    std::cout << "Connected to " << server_ip << ":" << port << std::endl;

    game::login::CGLogin login_req;
    login_req.set_username("codex");
    login_req.set_password("123456");
    if (send_packet(sock, PACKET_CG_LOGIN, login_req) == false)
    {
        std::cerr << "send login packet fail" << std::endl;
        close_socket(sock);
        cleanup_network();
        return 1;
    }

    WirePacket login_rsp_packet;
    if (recv_packet(sock, login_rsp_packet) == false)
    {
        std::cerr << "recv login packet fail" << std::endl;
        close_socket(sock);
        cleanup_network();
        return 1;
    }

    if (login_rsp_packet.header.m_nPacketId == PACKET_GC_LOGIN)
    {
        game::login::GCLogin login_rsp;
        if (login_rsp.ParseFromArray(login_rsp_packet.payload.data(), static_cast<int>(login_rsp_packet.payload.size())))
        {
            std::cout << "LoginRsp result_code=" << login_rsp.result_code()
                      << " message=" << login_rsp.message() << std::endl;
        }
    }

    game::chat::CGChat chat_req;
    chat_req.set_username("codex");
    chat_req.set_password("hello protobuf");
    if (send_packet(sock, PACKET_CG_CHAT, chat_req) == false)
    {
        std::cerr << "send chat packet fail" << std::endl;
        close_socket(sock);
        cleanup_network();
        return 1;
    }

    WirePacket chat_rsp_packet;
    if (recv_packet(sock, chat_rsp_packet) == false)
    {
        std::cerr << "recv chat packet fail" << std::endl;
        close_socket(sock);
        cleanup_network();
        return 1;
    }

    if (chat_rsp_packet.header.m_nPacketId == PACKET_GC_CHAT)
    {
        game::chat::GCChat chat_rsp;
        if (chat_rsp.ParseFromArray(chat_rsp_packet.payload.data(), static_cast<int>(chat_rsp_packet.payload.size())))
        {
            std::cout << "ChatRsp result_code=" << chat_rsp.result_code()
                      << " message=" << chat_rsp.message() << std::endl;
        }
    }

    close_socket(sock);
    cleanup_network();
    return 0;
}
