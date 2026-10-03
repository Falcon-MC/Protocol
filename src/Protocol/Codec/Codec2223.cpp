#include "Protocol/Codec/Codec2223.h"

#include "Protocol/MinecraftPackets.h"

namespace Codec2223 {

    std::shared_ptr<const ProtocolCodec> create() {
        ProtocolCapabilities capabilities;
        capabilities.mProtocolVersion = 2223;

        std::shared_ptr<ProtocolCodec> codec = std::make_shared<ProtocolCodec>(capabilities);
        MinecraftPackets::registerVanillaPackets(*codec);
        return codec;
    }

}
