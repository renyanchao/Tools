#ifndef _WIN32

#include "Epoll.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Epoll::~Epoll()
{
    stop();
}

bool Epoll::Init()
{
    if (!init_network()) {
        return false;
    }

    listen_fd = create_listen_socket(PORT);
    if (listen_fd == INVALID_SOCKET_VAL) {
        cleanup_network();
        return false;
    }

    epoll_event event = {};
    event.events = EPOLLIN;
    event.data.ptr = nullptr;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == SOCKET_ERROR_VAL) {
        std::cerr << "epoll_ctl() error: " << GET_LAST_ERROR() << std::endl;
        stop();
        return false;
    }

    std::cout << "epoll server listening on port " << PORT << std::endl;
    return true;
}

void Epoll::AcceptNewSession()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    socket_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd == INVALID_SOCKET_VAL) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            std::cerr << "accept() error: " << GET_LAST_ERROR() << std::endl;
        }
        return;
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close_socket(client_fd);
        return;
    }
    if (client_sessionlist.size() >= MAX_CLIENTS) {
        std::cerr << "Too many clients, rejecting new connection." << std::endl;
        close_socket(client_fd);
        return;
    }

    Session* pSession = nullptr;
    try {
        pSession = new Session(client_fd);
    }
    catch (...) {
        close_socket(client_fd);
        throw;
    }
    AddSession(pSession);
    std::cout << "Accept New client socketid = " << client_fd << std::endl;
}

void Epoll::Tick()
{
    ProcessInput();
    ProcessCommand();
    ProcessOutput();
    ProcessClose();
}

void Epoll::ProcessInput()
{
    if (epoll_fd == -1) return;

    for (const auto& sessionPtr : client_sessionlist) {
        Session* pSession = sessionPtr.get();
        if (pSession->IsDead()) continue;
        socket_t sock = pSession->GetSocket();
        epoll_event event = {};
        event.events = EPOLLIN;
        if (pSession->HasPendingOutput()) {
            event.events |= EPOLLOUT;
        }
        event.data.ptr = pSession;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock, &event) == SOCKET_ERROR_VAL) {
            std::cerr << "epoll_ctl() error: " << GET_LAST_ERROR() << std::endl;
            pSession->Close();
        }
    }

    epoll_event events[BUFFER_SIZE];
    int activity = epoll_wait(epoll_fd, events, BUFFER_SIZE, 1);
    if (activity == SOCKET_ERROR_VAL) {
        if (errno != EINTR) {
            std::cerr << "epoll_wait() error: " << GET_LAST_ERROR() << std::endl;
        }
        return;
    }

    for (int i = 0; i < activity; ++i) {
        if (events[i].data.ptr == nullptr && (events[i].events & EPOLLIN)) {
            std::cout << "New client connected." << std::endl;
            AcceptNewSession();
        }
    }

    // Session objects remain alive until ProcessClose, even if an fd is reused.
    for (int i = 0; i < activity; ++i) {
        Session* pSession = static_cast<Session*>(events[i].data.ptr);
        if (pSession == nullptr || pSession->IsDead()) continue;
        if (events[i].events & EPOLLERR) {
            pSession->Close();
            continue;
        }
        if (!(events[i].events & EPOLLIN)) {
            if (events[i].events & EPOLLHUP) pSession->Close();
            continue;
        }
        if (!pSession->ProcessInput()) {
            pSession->Close();
        }
    }

    for (int i = 0; i < activity; ++i) {
        Session* pSession = static_cast<Session*>(events[i].data.ptr);
        if (pSession == nullptr || pSession->IsDead()) continue;
        if (!(events[i].events & EPOLLOUT)) continue;
        if (!pSession->ProcessOutput()) {
            pSession->Close();
        }
    }
}

void Epoll::ProcessCommand()
{
    for (auto& sessionPtr : client_sessionlist)
    {
        Session* pSession = sessionPtr.get();
        if (pSession->IsDead()) continue;
        if (!pSession->ProcessCommand())
        {
            pSession->Close();
        }
    }
}

void Epoll::ProcessOutput()
{
    // Output is handled by ProcessInput() through write readiness.
}

void Epoll::ProcessClose()
{
    for (auto it = client_sessionlist.begin(); it != client_sessionlist.end();)
    {
        Session* pSession = it->get();
        if (pSession->IsDead())
        {
            // close() already removed the socket from epoll.
            socket_t socketId = pSession->GetSocket();
            it = client_sessionlist.erase(it);
            std::cout << "Session socket_id = " << socketId << " will close" << std::endl;
        }
        else
        {
            ++it;
        }
    }
}

void Epoll::AddSession(Session* pSession)
{
    if (pSession == nullptr)
    {
        return;
    }
    for (const auto& sessionPtr : client_sessionlist) {
        if (sessionPtr.get() == pSession) return;
    }
    std::unique_ptr<Session> sessionPtr(pSession);
    socket_t sock = pSession->GetSocket();
    int flags = fcntl(sock, F_GETFL, 0);
    epoll_event event = {};
    event.events = EPOLLIN;
    if (pSession->HasPendingOutput()) event.events |= EPOLLOUT;
    event.data.ptr = pSession;
    if (pSession->IsDead()) return;
    if (client_sessionlist.size() >= MAX_CLIENTS || flags == -1 ||
        fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1 ||
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == SOCKET_ERROR_VAL) {
        std::cerr << "AddSession() failed: " << GET_LAST_ERROR() << std::endl;
        pSession->Close();
        return;
    }
    client_sessionlist.push_back(std::move(sessionPtr));
    std::cout << "Accept New client connected. Total clients: " << client_sessionlist.size() << std::endl;
}

bool Epoll::init_network() {
    if (epoll_fd != -1) return false;
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1() error: " << GET_LAST_ERROR() << std::endl;
        return false;
    }
    return true;
}

void Epoll::cleanup_network() {
    if (epoll_fd != -1) {
        close(epoll_fd);
        epoll_fd = -1;
    }
}

socket_t Epoll::create_listen_socket(int port) {
    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VAL) {
        std::cerr << "socket() error: " << GET_LAST_ERROR() << std::endl;
        return INVALID_SOCKET_VAL;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR_VAL) {
        close_socket(listen_fd);
        return INVALID_SOCKET_VAL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "bind() error: " << GET_LAST_ERROR() << std::endl;
        close_socket(listen_fd);
        return INVALID_SOCKET_VAL;
    }

    if (listen(listen_fd, SOMAXCONN) == SOCKET_ERROR_VAL) {
        std::cerr << "listen() error: " << GET_LAST_ERROR() << std::endl;
        close_socket(listen_fd);
        return INVALID_SOCKET_VAL;
    }

    int flags = fcntl(listen_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close_socket(listen_fd);
        return INVALID_SOCKET_VAL;
    }
    return listen_fd;
}

void Epoll::stop()
{
    if (listen_fd != INVALID_SOCKET_VAL) {
        close_socket(listen_fd);
        listen_fd = INVALID_SOCKET_VAL;
    }
    for (const auto& sessionPtr : client_sessionlist) {
        Session* pSession = sessionPtr.get();
        if (!pSession->IsDead()) pSession->Close();
    }
    client_sessionlist.clear();
    cleanup_network();
}

#endif
