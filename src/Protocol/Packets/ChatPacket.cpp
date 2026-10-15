#include "Protocol/Packets/ChatPacket.h"

#include "Protocol/NetworkPacketHandler.h"

void ChatPacket::write(BinaryStream &stream, const PacketCodecContext &context) const {
    mSignedChat.write(stream);
    stream.putBool(mHasFormatting);
    if (mHasFormatting)
        mFormatting.write(stream);
}

void ChatPacket::read(ReadOnlyBinaryStream &stream, const PacketCodecContext &context) {
    mSignedChat.read(stream);
    mHasFormatting = stream.getBool();
    if (mHasFormatting)
        mFormatting.read(stream);
}

void ChatPacket::handle(const NetworkIdentifier &id, NetworkPacketHandler &handler) const {
    handler.handle(id, *this);
}
