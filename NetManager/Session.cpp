#include"Session.h"
#include"Packet.h"

void Session::ReadPacket(Packet* pPacket)
{
    PacketHeader header;

    if (m_ReadStream.Peek(&header, sizeof(header)) == false)
    {
        return;
    }
    if(m_ReadStream.GetUsedSize() < sizeof(header) + header.m_nPacketSize)
    {
        return;
    }
    m_ReadStream.Read(&header, sizeof(header));
    m_ReadStream.Read(pPacket->Buff, header.m_nPacketSize);
    
    

}

void Session::WritePacket(Packet* pPacket)
{

}


int main()
{
    Socket_IOStream stream(30);
    
    Packet_2 packet;
    strncpy(packet.szName, "012345678901234567890123456789", sizeof(packet.szName));
    char buff[1024] = {0};

    stream.Write(&packet, sizeof(packet));
    
    memset(buff, 0, sizeof(buff));
    stream.Read(buff, sizeof(packet));

    stream.Write(&packet, sizeof(packet));
    memset(buff, 0, sizeof(buff));
    stream.Read(buff, sizeof(packet));
    Packet_2* p = (Packet_2*)buff;
    std::cout<<"p.szName = "<<p->szName<<std::endl;

}