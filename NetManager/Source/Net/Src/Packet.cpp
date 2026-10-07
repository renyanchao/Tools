#include <Packet.h>

#include "Chat.pb.h"
#include "Login.pb.h"

void RegisterPacket()
{
    PacketFactory::Register<game::login::CGLogin>(PacketId::PACKET_CG_LOGIN);
    PacketFactory::Register<game::chat::CGChat>(PacketId::PACKET_CG_CHAT);
}
