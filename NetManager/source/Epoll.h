#include <iostream>
#include <cstring>
#include <vector>

#include "Session.h"

#include <algorithm>

const int PORT = 8888;
const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = FD_SETSIZE - 10;  // 为监听套接字留出空间

#ifndef WIN32
class Epoll
{
public:
    bool Init();
    void Tick();
    void AcceptNewSession();
    void AddSession(socket_t client_fd);
private:
    bool init_network();
    void cleanup_network();
    socket_t create_listen_socket(int port);
    void stop();
    
private:
    socket_t server_fd;
    socket_t epoll_fd;
    std::vector<socket_t> client_sockets;
};
#endif