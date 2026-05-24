#ifdef _WIN32

#include "Select.h"
#include "WS2tcpip.h"
bool Select::Init() 
{
    if (!init_network()) {
        return false;
    }

    listen_fd = create_listen_socket(PORT);
    if (listen_fd == INVALID_SOCKET_VAL) {
        cleanup_network();
        return false;
    }

    std::cout << "selector server listening on port " << PORT << std::endl;
    return true;
}

void Select::AcceptNewSession()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    socket_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd == INVALID_SOCKET_VAL) {
        std::cerr << "accept() error: " << GET_LAST_ERROR() << std::endl;
        return;
    }
    u_long mode = 1;
    ioctlsocket(client_fd, FIONBIO, &mode);
    if (client_sessionlist.size() >= MAX_CLIENTS) {
        std::cerr << "Too many clients, rejecting new connection." << std::endl;
        close_socket(client_fd);
        return;
    } 
    Session* pSession = new Session(client_fd);
    if(pSession == nullptr)
    {
        std::cerr << "New Session() fail." << std::endl;
        close_socket(client_fd);
        return;
    }
    client_sessionlist.push_back(pSession);
    std::cout << "Accept New client socketid = " << client_fd << std::endl;
    std::cout << "Accept New client connected. Total clients: " << client_sessionlist.size() << std::endl;
    
}

void Select::Tick() //Tick Once
{
    ProcessInput();
    ProcessCommand();
    ProcessOutput();
    ProcessClose();
}
void Select::ProcessInput()
{
    fd_set read_fds;
    int max_fd = static_cast<int>(listen_fd);  // 仅用于 Linux/Unix，Windows 忽略


    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);

    // 记录最大文件描述符（Linux 需要）
    int current_max_fd = static_cast<int>(listen_fd);
    for(Session* pSession : client_sessionlist){
        socket_t sock = pSession->GetSocket();
        FD_SET(sock, &read_fds);
        if (static_cast<int>(sock) > current_max_fd) {
            current_max_fd = static_cast<int>(sock);
        }
    }

    // 调用 select
    // Windows: 第一个参数被忽略，但传递 0 或 current_max_fd+1 均可
    int activity = select(current_max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
    if (activity == SOCKET_ERROR_VAL) {
        std::cerr << "select() error: " << GET_LAST_ERROR() << std::endl;
        //break;
        return;
    }

    // 1. 处理新连接
    if (FD_ISSET(listen_fd, &read_fds)) 
    {
        std::cout << "New client connected." << std::endl;
        AcceptNewSession();
    }

    // 2. 处理现有客户端的数据
    for(auto it = client_sessionlist.begin(); it != client_sessionlist.end(); it++)
    {
        Session* pSession = *it;
        if(pSession == nullptr)continue;
        if (FD_ISSET(pSession->GetSocket(), &read_fds) == false)continue;
        if (pSession->ProcessInput() == false)
        {
            pSession->Close();
        }
    }

}
void Select::ProcessCommand()
{
    for(auto it = client_sessionlist.begin(); it != client_sessionlist.end(); it++)
    {
        Session* pSession = *it;
        if (pSession->ProcessCommand() == false)
        {
            pSession->Close();
        }
    }
}
void Select::ProcessOutput()
{
    for(auto it = client_sessionlist.begin(); it != client_sessionlist.end(); it++)
    {
        Session* pSession = *it;
        if (pSession->ProcessOutput() == false)
        {
            pSession->Close();
        }
    }
}
void Select::ProcessClose()
{
    for (auto it = client_sessionlist.begin(); it != client_sessionlist.end();)
    {
        Session* pSession = *it;
        if (pSession->IsDead())
        {
            it = client_sessionlist.erase(it);
            std::cout << "Session socket_id = " << pSession->GetSocket() << "will close" << std::endl;
        }
        else
        {
            it++;
        }
    }
}


void Select::AddSession(Session* pSession)
{
    if(pSession == nullptr)
    {
        return;
    }
    client_sessionlist.push_back(pSession);
    std::cout << "Accept New client connected. Total clients: " << client_sessionlist.size() << std::endl;
}

bool Select::init_network() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
#endif
    return true;
}

void Select::cleanup_network() {
#ifdef _WIN32
    WSADATA wsaData;
    WSACleanup();
#endif
}

socket_t Select::create_listen_socket(int port) {
    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VAL) {
        std::cerr << "socket() error: " << GET_LAST_ERROR() << std::endl;
        return INVALID_SOCKET_VAL;
    }

    // 允许端口重用（避免 TIME_WAIT 问题）
    int opt = 1;
#ifdef _WIN32
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

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

    // 非阻塞模式（可选，但建议配合 select 时设置）
#ifndef _WIN32
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);
#else
    // Windows 下设置非阻塞
    u_long mode = 1;
    ioctlsocket(listen_fd, FIONBIO, &mode);
#endif

    return listen_fd;
}

void Select::stop()
{
    // 清理资源
    close_socket(listen_fd);
    for(Session* pSession : client_sessionlist){
        socket_t sock = pSession->GetSocket();
        close_socket(sock);
    }
    cleanup_network();
}

#endif


