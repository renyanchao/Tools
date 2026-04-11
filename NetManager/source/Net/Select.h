#include <iostream>
#include <cstring>
#include <vector>

#include "Session.h"

#include <algorithm>

const int PORT = 8888;
const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = FD_SETSIZE - 10;  // 为监听套接字留出空间


class Select
{
public:
    bool Init();
    void ProcessInput();
    void ProcessCommand();
    void ProcessOutput();
    void ProcessClose();
    void Tick();
    void AcceptNewSession();
    void AddSession(Session* pSession);
private:
    bool init_network();
    void cleanup_network();
    socket_t create_listen_socket(int port);
    void stop();
    
private:
    socket_t listen_fd;

    std::vector<Session*> client_sessionlist;
};