#pragma once

#include <cstdint>
#include <string>
#include <utility>

enum PacketId : int32_t
{
    PACKET_CG_LOGIN = 1,
    PACKET_GC_LOGIN = 1001,
    PACKET_CG_CHAT = 2,
    PACKET_GC_CHAT = 1002,
};

struct PacketHeader
{
    int32_t m_nPacketId = 0;
    int32_t m_nPacketSize = 0;
};

class PacketData
{
public:
    PacketData() = default;

    PacketData(int32_t packetId, std::string PacketData)
        : m_nPacketId(packetId), m_PacketData(std::move(PacketData))
    {
    }

    int32_t GetPacketId() const { return m_nPacketId; }
    int32_t GetPacketSize() const { return static_cast<int32_t>(m_PacketData.size()); }
    const std::string& GetPacketData() const { return m_PacketData; }

    void SetPacketId(int32_t packetId) { m_nPacketId = packetId; }
    void SetPayload(std::string payloadData) { m_PacketData = std::move(payloadData); }

private:
    int32_t m_nPacketId = 0;
    std::string m_PacketData;
};


#include <google/protobuf/message.h>
#include <unordered_map>
using Packet = google::protobuf::Message;
class PacketFactory
{
public:
    template<typename T>
    static void Register(int32_t packetId)
    {
        GetRegisterMap()[packetId] = &T::default_instance();
    }
    static std::unique_ptr<Packet> CreatePacket(int32_t packetId)
    {
        auto it = GetRegisterMap().find(packetId);
        if (it == GetRegisterMap().end())
        {
            return nullptr;
        }
        return std::unique_ptr<Packet>(it->second->New());
    }
    
    
private:
    static std::unordered_map<int32_t, const Packet*>& GetRegisterMap()
    {
        static std::unordered_map<int32_t, const Packet*> m_packets;
        return m_packets;
    }
};

void RegisterPacket();
