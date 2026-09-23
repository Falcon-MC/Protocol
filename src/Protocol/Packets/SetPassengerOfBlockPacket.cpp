#include "Protocol/Packets/SetPassengerOfBlockPacket.h"

#include "Protocol/NetworkPacketHandler.h"

SetPassengerOfBlockPacket::SetPassengerOfBlockPacket() = default;

void SetPassengerOfBlockPacket::write(BinaryStream &stream, const PacketCodecContext &context) const {
    stream.putVarLong(mPassengerUniqueId);

    stream.putOptionalPresent(mHasData);
    if (mHasData) {
        stream.putBlockPosition(mBlockPosition);
        stream.putVector3f(mOffset);
        stream.putLFloat(mRotation);
        stream.putLFloat(mRotationLimit);
        stream.putByte((unsigned char) mEmoteType);
    }
}

void SetPassengerOfBlockPacket::read(ReadOnlyBinaryStream &stream, const PacketCodecContext &context) {
    mPassengerUniqueId = stream.getVarLong();

    mHasData = stream.getOptionalPresent();
    if (mHasData) {
        mBlockPosition = stream.getBlockPosition();
        mOffset = stream.getVector3f();
        mRotation = stream.getLFloat();
        mRotationLimit = stream.getLFloat();
        mEmoteType = (EmoteType) stream.getByte();
    }
}

void SetPassengerOfBlockPacket::handle(const NetworkIdentifier &id, NetworkPacketHandler &handler) const {
    handler.handle(id, *this);
}
