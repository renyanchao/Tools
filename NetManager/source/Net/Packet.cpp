#include <Packet.h>

#include "Chat.Proto.pb.h"
#include "Login.Proto.pb.h"

void RegisterPacket()
{
    PacketFactory::Register<game::login::CGLogin>(PacketId::PACKET_CG_LOGIN);
    PacketFactory::Register<game::chat::CGChat>(PacketId::PACKET_CG_CHAT);
}
