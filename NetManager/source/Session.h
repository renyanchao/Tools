#include "SocketStream.h"


class Packet;
class Session
{
    Session(int32_t nBuffSize): m_ReadStream(nBuffSize), m_WriteStream(nBuffSize){}

    void ReadPacket(Packet* pPacket);
    void WritePacket(Packet* pPacket);


private:
    Socket_IOStream m_ReadStream;
    Socket_IOStream m_WriteStream;
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