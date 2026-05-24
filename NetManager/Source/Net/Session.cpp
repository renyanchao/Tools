#include "Session.h"

#include "Packet.h"
#include "Chat.pb.h"
#include "Login.pb.h"

#include <iostream>
#include <memory>
#include <utility>

bool Session::ProcessInput()
{
    return m_ReadStream.Fill(&m_fd);
}

bool Session::ProcessCommand()
{
    while (true)
    {
        std::unique_ptr<PacketData> packet = ReadPacket();
        if (packet == nullptr)
        {
            break;
        }

        if (HandlePacket(*packet) == false)
        {
            return false;
        }
    }

    return true;
}

bool Session::ProcessOutput()
{
    return m_WriteStream.Flush(&m_fd);
}

void Session::Close()
{
    m_IsDead = true;
    close_socket(m_fd);
    std::cout << "Session mark needclose. And Close SocketID = " << m_fd << std::endl;
}

std::unique_ptr<PacketData> Session::ReadPacket()
{
    PacketHeader header;
    if (m_ReadStream.Peek(&header, sizeof(header)) == false)
    {
        return nullptr;
    }

    if (header.m_nPacketSize < 0)
    {
        return nullptr;
    }

    if (m_ReadStream.GetUsedSize() < sizeof(header) + static_cast<size_t>(header.m_nPacketSize))
    {
        return nullptr;
    }

    m_ReadStream.Read(&header, sizeof(header));

    std::string payload;
    payload.resize(static_cast<size_t>(header.m_nPacketSize));
    if (header.m_nPacketSize > 0)
    {
        m_ReadStream.Read(&payload[0], static_cast<size_t>(header.m_nPacketSize));
    }

    return std::make_unique<PacketData>(header.m_nPacketId, std::move(payload));
}

bool Session::WritePacket(const PacketData& packet)
{
    PacketHeader header;
    header.m_nPacketId = packet.GetPacketId();
    header.m_nPacketSize = packet.GetPacketSize();

    const int32_t totalSize = static_cast<int32_t>(sizeof(header)) + header.m_nPacketSize;
    if (m_WriteStream.GetFreeSize() < static_cast<size_t>(totalSize))
    {
        if (m_WriteStream.Resize(totalSize) == false)
        {
            return false;
        }
    }

    if (m_WriteStream.GetFreeSize() < static_cast<size_t>(totalSize))
    {
        return false;
    }

    m_WriteStream.Write(&header, sizeof(header));
    if (header.m_nPacketSize > 0)
    {
        m_WriteStream.Write(packet.GetPacketData().data(), static_cast<size_t>(header.m_nPacketSize));
    }

    return true;
}

bool Session::HandlePacket(const PacketData& packetData)
{
    std::cout << "Session HandlePacket socket=" << m_fd
              << " packetId=" << packetData.GetPacketId()
              << " payloadSize=" << packetData.GetPacketSize() << std::endl;
    
    std::unique_ptr<Packet> packet = PacketFactory::CreatePacket(packetData.GetPacketId());
    if (packet == nullptr)
    {
        return false;
    }
    bool ret = packet->ParseFromArray(packetData.GetPacketData().data(), static_cast<int>(packetData.GetPacketSize()) );
    if (ret == false)
    {
        std::cerr << "Parse Packet fail" << std::endl;
        return  false;
    }
    std::cout << "Receive Packet PacketId =" << packetData.GetPacketId() << std::endl;
    if (packetData.GetPacketId() == PacketId::PACKET_CG_LOGIN)
    {
        // std::unique_ptr<game::login::CGLogin> rep = std::static_cast<game::login::CGLogin>(Packet);
        //
        // game::login::GCLogin rsp;
        // rsp.set_result_code(0);
        // rsp.set_message("login success: " + rep->username());
    }
    
    // if (packet.GetPacketId() == PACKET_CG_LOGIN)
    // {
    //     game::login::CGLogin req;
    //     if (req.ParseFromArray(packet.GetPacketData().data(), packet.GetPacketSize()) == false)
    //     {
    //         std::cerr << "Parse CGLogin fail" << std::endl;
    //         return false;
    //     }
    //
    //     std::cout << "Receive CGLogin username=" << req.username() << std::endl;
    //
    //     game::login::GCLogin rsp;
    //     rsp.set_result_code(0);
    //     rsp.set_message("login success: " + req.username());
    //
    //     std::string payload;
    //     if (rsp.SerializeToString(&payload) == false)
    //     {
    //         return false;
    //     }
    //
    //     return WritePacket(Packet(PACKET_GC_LOGIN, std::move(payload)));
    // }
    //
    // if (packet.GetPacketId() == PACKET_CG_CHAT)
    // {
    //     game::chat::CGChat req;
    //     if (req.ParseFromArray(packet.GetPacketData().data(), packet.GetPacketSize()) == false)
    //     {
    //         std::cerr << "Parse CGChat fail" << std::endl;
    //         return false;
    //     }
    //
    //     std::cout << "Receive CGChat username=" << req.username()
    //               << " password=" << req.password() << std::endl;
    //
    //     game::chat::GCChat rsp;
    //     rsp.set_result_code(0);
    //     rsp.set_message(req.username() + ": " + req.password());
    //
    //     std::string payload;
    //     if (rsp.SerializeToString(&payload) == false)
    //     {
    //         return false;
    //     }
    //
    //     return WritePacket(Packet(PACKET_GC_CHAT, std::move(payload)));
    // }
    
    return false;
}
