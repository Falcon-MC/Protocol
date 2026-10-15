#include "Protocol/Codec/Codec2235.h"

#include "Protocol/MinecraftPackets.h"

namespace Codec2235 {

    std::shared_ptr<const ProtocolCodec> create() {
        ProtocolCapabilities capabilities;
        capabilities.mProtocolVersion = 2235;

        std::shared_ptr<ProtocolCodec> codec = std::make_shared<ProtocolCodec>(capabilities);
        MinecraftPackets::registerVanillaPackets(*codec);
        codec->removePacket(MinecraftPacketIds::AddVolumeEntity);
        codec->removePacket(MinecraftPacketIds::RemoveVolumeEntity);
        return codec;
    }

}
