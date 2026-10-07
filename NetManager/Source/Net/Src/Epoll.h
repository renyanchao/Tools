#pragma once
#ifndef _WIN32
#include <iostream>
#include <cstring>
#include <vector>
#include <memory>

#include "Session.h"

#include <algorithm>
#include <sys/epoll.h>
const int PORT = 8888;
const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = 65536;

class Epoll
{
public:
    Epoll() = default;
    ~Epoll();
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;
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
    socket_t listen_fd = INVALID_SOCKET_VAL;
    int epoll_fd = -1;
    std::vector<std::unique_ptr<Session>> client_sessionlist;
};
#endif
