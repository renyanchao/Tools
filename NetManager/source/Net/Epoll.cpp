#include "Epoll.h"
#ifndef WIN32
// 设置文件描述符为非阻塞模式
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


bool Epoll::Init()
{
        if (!init_network()) {
        return 1;
    }

    server_fd = create_listen_socket(PORT);
    if (server_fd == INVALID_SOCKET_VAL) {
        cleanup_network();
        return 1;
    }

    std::cout << "selector server listening on port " << PORT << std::endl;
}

socket_t Epoll::create_listen_socket(int port)
{
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        std::cout<<"socket creation failed"<<std::endl;
        return INVALID_SOCKET_VAL;
    }
    if(set_nonblocking(server_fd) < 0)
    {
        std::cout<<"socket set_nonblocking server_fd error" <<std::endl;     
        return INVALID_SOCKET_VAL;   
    }
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)))
    {
        std::cout<<"socket setsockopt server_fd error" <<std::endl;     
        return INVALID_SOCKET_VAL;
    }

    memset(&server_addr, 9, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout<<"bind fail"<<std::endl;
        return INVALID_SOCKET_VAL;
    }

    int32_t LISTEN_QUENE_SIZE = 2;
    if(listen(server_fd, LISTEN_QUENE_SIZE) < 0)
    {
        std::cout<<"listen fail"<<std::endl;
        return INVALID_SOCKET_VAL;
    }
    
    // epoll_fd = epoll_create(0);
    // if(epoll_fd <)

}
#endif