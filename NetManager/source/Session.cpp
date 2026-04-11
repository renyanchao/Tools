#include"Session.h"
#include"Packet.h"

bool Session::ProcessInput()
{

    char buffer[1024] = {0};
    int len = recv(m_fd, buffer, sizeof(buffer) - 1, 0);
    if(len == 0)
    {
        std::cout << "Client disconnected gracefully." << std::endl;
        return false;
    }
    if(len < 0)
    {
        std::cerr << "recv() error: " << GET_LAST_ERROR() << std::endl;
        return false;
    }
    std::cout<<"Session ProcessInput:"<<m_fd<<" bytesize="<<len<<std::endl;
    m_ReadStream.Write(buffer, len);
    return true;
}
bool Session::ProcessCommand()
{
    if (m_ReadStream.GetUsedSize() <= 0)
    {
        return true;
    }

    // if(error)
    // {
    //     return false;
    // }

    char buff[1024] = {0};
    int len = m_ReadStream.Read(buff, sizeof(buff));
    m_WriteStream.Write(buff, len);
    std::cout<<"Session ProcessCommand:"<<m_fd<<" bytesize="<<len<<std::endl;
    return true;
}
bool Session::ProcessOutput()
{
    char buff[1024] = {0};
    int len = m_WriteStream.Read(buff, sizeof(buff));
    if(len <= 0)
    {
        return true;
    }
    std::cout<<"Session ProcessOutput:"<<m_fd<<" bytesize="<<len<<std::endl;
    send(m_fd, buff, len, 0);
    return true;
}


Packet* Session::ReadPacket()
{
    PacketHeader header;

    if (m_ReadStream.Peek(&header, sizeof(header)) == false)
    {
        return nullptr;
    }
    if(m_ReadStream.GetUsedSize() < sizeof(header) + header.m_nPacketSize)
    {
        return nullptr;
    }
    m_ReadStream.Read(&header, sizeof(header));
    // m_ReadStream.Read(pPacket->Buff, header.m_nPacketSize);
    Packet* pPacket = new Packet;
    return pPacket;
}


void Session::WritePacket(Packet* pPacket)
{
    char buff[1024] = {0};
    PacketHeader header;

    int32_t nLen = sizeof(buff) + sizeof(header);

    if(m_WriteStream.GetFreeSize() < nLen)
    {
        m_WriteStream.Resize(nLen);
    }
    if(m_WriteStream.GetFreeSize() < nLen)
    {
        return;
    }
    m_WriteStream.Write(&header, sizeof(header));
    m_WriteStream.Write(&buff, sizeof(buff));

}


// int main()
// {
//     Socket_IOStream stream(30);
    
//     Packet_2 packet;
//     strncpy(packet.szName, "012345678901234567890123456789", sizeof(packet.szName));
//     char buff[1024] = {0};

//     stream.Write(&packet, sizeof(packet));
    
//     memset(buff, 0, sizeof(buff));
//     stream.Read(buff, sizeof(packet));

//     stream.Write(&packet, sizeof(packet));
//     memset(buff, 0, sizeof(buff));
//     stream.Read(buff, sizeof(packet));
//     Packet_2* p = (Packet_2*)buff;
//     std::cout<<"p.szName = "<<p->szName<<std::endl;

// }