#include "SocketStream.h"

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
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/epoll.h>
    #include <errno.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL   (-1)
    #define close_socket(s)    close(s)
    #define GET_LAST_ERROR()   errno
#endif

class Packet;
class Session
{
public:
    Session(socket_t fd, int32_t nBuffSize = 1024 * 1024): m_ReadStream(nBuffSize), m_WriteStream(nBuffSize)
    {
        m_fd = fd;
    }

    bool ProcessInput();
    bool ProcessCommand();
    bool ProcessOutput(); 


    Packet* ReadPacket();
    void WritePacket(Packet* pPacket);

    socket_t GetSocket()const{return m_fd;}
    Socket_IOStream& GetReadStream(){return m_ReadStream;}
    Socket_IOStream& GetWriteStream(){return m_WriteStream;}
    
private:
    Socket_IOStream m_ReadStream;
    Socket_IOStream m_WriteStream;

    socket_t m_fd = -1;
};



struct Packet_1
{
    int a = 0;
    int b = 1;
    float c = 2.2;
    char szName[16];
};

struct Packet_2
{
    char szName[24];
};