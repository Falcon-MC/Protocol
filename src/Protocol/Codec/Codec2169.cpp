#include "Protocol/Codec/Codec2169.h"

#include "Protocol/MinecraftPackets.h"

namespace Codec2169 {

    std::shared_ptr<const ProtocolCodec> create() {
        ProtocolCapabilities capabilities;
        capabilities.mProtocolVersion = 2169;
        capabilities.mDoubledPresence = true;
        capabilities.mBossEventPlayerId = true;
        capabilities.mItemUseHand = false;
        capabilities.mCameraStartingRotation = false;
        capabilities.mAttributeNoiseAlignment = false;
        capabilities.mMoveDeltaTicks = false;
        capabilities.mPlaySoundRangeAndPosition = false;
        capabilities.mSubChunkHeightMapRows = false;
        capabilities.mDiagnosticsActorPosition = false;

        std::shared_ptr<ProtocolCodec> codec = std::make_shared<ProtocolCodec>(capabilities);
        MinecraftPackets::registerVanillaPackets(*codec);
        codec->removePacket(MinecraftPacketIds::SetPlayerFurnaceOptions);
        codec->removePacket(MinecraftPacketIds::RecordStarted);
        return codec;
    }

}
