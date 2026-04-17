#pragma once

struct PacketHeader
{
    int32_t m_nPacketId;
    int32_t m_nPacketSize;
};

#define PACKET_BUFF_SIZE 1024

struct PacketBuffPtr
{
public:
    char* Buff = new char[PACKET_BUFF_SIZE];
};
class Packet : public PacketBuffPtr
{

};