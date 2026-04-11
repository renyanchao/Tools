// 文件名: echo_client.cpp
// 编译:
//   Linux:   g++ -std=c++11 echo_client.cpp -o client
//   Windows: g++ -std=c++11 echo_client.cpp -lws2_32 -o client.exe

#include <iostream>
#include <cstring>
#include <string>

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
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL   (-1)
    #define close_socket(s)    close(s)
    #define GET_LAST_ERROR()   errno
#endif

const int BUFFER_SIZE = 1024;
const char* DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_PORT = 8888;

bool init_network() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
#endif
    return true;
}

void cleanup_network() {
#ifdef _WIN32
    WSACleanup();
#endif
}

socket_t connect_to_server(const std::string& ip, int port) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL) {
        std::cerr << "socket() error: " << GET_LAST_ERROR() << std::endl;
        return INVALID_SOCKET_VAL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "inet_pton() error: invalid IP address" << std::endl;
        close_socket(sock);
        return INVALID_SOCKET_VAL;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "connect() error: " << GET_LAST_ERROR() << std::endl;
        close_socket(sock);
        return INVALID_SOCKET_VAL;
    }

    return sock;
}

int main(int argc, char* argv[]) {
    std::string server_ip = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    if (argc >= 2) server_ip = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    if (!init_network()) {
        return 1;
    }

    socket_t sock = connect_to_server(server_ip, port);
    if (sock == INVALID_SOCKET_VAL) {
        cleanup_network();
        return 1;
    }

    std::cout << "Connected to " << server_ip << ":" << port << std::endl;
    std::cout << "Type messages to send to server. Enter 'quit' or 'exit' to disconnect." << std::endl;

    char buffer[BUFFER_SIZE];
    while (true) {
        std::cout << "> ";
        std::cout.flush();

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }

        // 去除末尾换行符
        buffer[strcspn(buffer, "\n")] = '\0';

        std::string input(buffer);
        if (input == "quit" || input == "exit") {
            break;
        }

        // 发送数据到服务器
        int bytes_sent = send(sock, buffer, strlen(buffer), 0);
        if (bytes_sent == SOCKET_ERROR_VAL) {
            std::cerr << "send() error: " << GET_LAST_ERROR() << std::endl;
            break;
        }

        // 接收回显数据
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Server closed the connection." << std::endl;
            } else {
                std::cerr << "recv() error: " << GET_LAST_ERROR() << std::endl;
            }
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Echo: " << buffer << std::endl;
    }

    close_socket(sock);
    cleanup_network();
    return 0;
}