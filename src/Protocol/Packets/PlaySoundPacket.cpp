#include "Protocol/Packets/PlaySoundPacket.h"

#include "Protocol/NetworkPacketHandler.h"

PlaySoundPacket::PlaySoundPacket() = default;

void PlaySoundPacket::write(BinaryStream &stream, const PacketCodecContext &context) const {
    stream.putString(mSound);

    Vector3i blockPosition((int32_t) (mPosition.x * 8.0f), (int32_t) (mPosition.y * 8.0f), (int32_t) (mPosition.z * 8.0f));
    stream.putBlockPosition(blockPosition);

    stream.putLFloat(mVolume);
    stream.putLFloat(mPitch);

    stream.putVarInt(mLoopCount);

    const bool rangeAndPosition = context.getCapabilities().mPlaySoundRangeAndPosition;
    if (rangeAndPosition)
        stream.putBool(mBypassListenerRangeCheck);

    stream.putOptionalPresent(mHasServerSoundHandle);
    if (mHasServerSoundHandle)
        stream.putLLong((uint64_t) mServerSoundHandle);

    if (rangeAndPosition) {
        stream.putOptionalPresent(mHasPlaybackPosition);
        if (mHasPlaybackPosition)
            stream.putLFloat(mPlaybackPositionSeconds);
    }

    if (context.getCapabilities().mPlaySoundStartDelay) {
        stream.putOptionalPresent(mHasStartDelay);
        if (mHasStartDelay)
            stream.putLFloat(mStartDelaySeconds);
    }
}

void PlaySoundPacket::read(ReadOnlyBinaryStream &stream, const PacketCodecContext &context) {
    mSound = stream.getString();

    Vector3i blockPosition = stream.getBlockPosition();
    mPosition = Vector3f(blockPosition.x / 8.0f, blockPosition.y / 8.0f, blockPosition.z / 8.0f);

    mVolume = stream.getLFloat();
    mPitch = stream.getLFloat();

    mLoopCount = stream.getVarInt();

    const bool rangeAndPosition = context.getCapabilities().mPlaySoundRangeAndPosition;
    if (rangeAndPosition)
        mBypassListenerRangeCheck = stream.getBool();

    mHasServerSoundHandle = stream.getOptionalPresent();
    if (mHasServerSoundHandle)
        mServerSoundHandle = (int64_t) stream.getLLong();

    mHasPlaybackPosition = rangeAndPosition && stream.getOptionalPresent();
    if (mHasPlaybackPosition)
        mPlaybackPositionSeconds = stream.getLFloat();

    mHasStartDelay = context.getCapabilities().mPlaySoundStartDelay && stream.getOptionalPresent();
    if (mHasStartDelay)
        mStartDelaySeconds = stream.getLFloat();
}

void PlaySoundPacket::handle(const NetworkIdentifier &id, NetworkPacketHandler &handler) const {
    handler.handle(id, *this);
}
