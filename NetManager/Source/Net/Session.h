#pragma once

#include <memory>

#include "SocketStream.h"

class PacketData;
class Session
{
public:
    Session(socket_t fd, int32_t nBuffSize = 1024 * 1024)
        : m_ReadStream(nBuffSize), m_WriteStream(nBuffSize)
    {
        m_fd = fd;
    }

    bool ProcessInput();
    bool ProcessCommand();
    bool ProcessOutput();
    void Close();
    bool IsDead() const { return m_IsDead; }
    std::unique_ptr<PacketData> ReadPacket();
    bool WritePacket(const PacketData& packet);

    socket_t GetSocket() const { return m_fd; }

private:
    bool HandlePacket(const PacketData& packet);

private:
    SocketStream m_ReadStream;
    SocketStream m_WriteStream;

    socket_t m_fd = -1;
    bool m_IsDead = false;
};
