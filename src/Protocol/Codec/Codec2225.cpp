#include "Protocol/Codec/Codec2225.h"

#include "Protocol/MinecraftPackets.h"

namespace Codec2225 {

    ProtocolCapabilities capabilities() {
        ProtocolCapabilities capabilities;
        capabilities.mProtocolVersion = 2225;
        capabilities.mManagedChat = false;
        capabilities.mCommandOutputRawMessage = false;
        capabilities.mCommandBlockRawMessage = false;
        capabilities.mKeepStructureVoid = false;
        capabilities.mTypedAttributeValues = false;
        return capabilities;
    }

    void removeLaterPackets(ProtocolCodec &codec) {
        codec.removePacket(MinecraftPacketIds::Chat);
    }

    std::shared_ptr<const ProtocolCodec> create() {
        std::shared_ptr<ProtocolCodec> codec = std::make_shared<ProtocolCodec>(capabilities());
        MinecraftPackets::registerVanillaPackets(*codec);
        removeLaterPackets(*codec);
        return codec;
    }

}
