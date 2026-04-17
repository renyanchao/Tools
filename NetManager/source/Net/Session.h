#pragma once

#include "SocketStream.h"


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
    void Close();
    bool IsDead()const { return m_IsDead; }
    Packet* ReadPacket();
    void WritePacket(Packet* pPacket);

    socket_t GetSocket()const{return m_fd;}
private:

    
private:
    Socket_IStream m_ReadStream;
    Socket_OStream m_WriteStream;

    socket_t m_fd = -1;
    bool m_IsDead = false;
};



//struct Packet_1
//{
//    int a = 0;
//    int b = 1;
//    float c = 2.2;
//    char szName[16];
//};
//
//struct Packet_2
//{
//    char szName[24];
//};