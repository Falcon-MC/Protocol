#include "Protocol/PacketCodecContext.h"

#include "Protocol/Codec/ProtocolCodecRegistry.h"

#include <utility>

PacketCodecContext::PacketCodecContext(const BlockDefinitionRegistry &blockDefinitions,
                                       const ItemDefinitionRegistry &itemDefinitions)
        : mBlockDefinitions(blockDefinitions), mItemDefinitions(itemDefinitions) {}

PacketCodecContext::PacketCodecContext(const BlockDefinitionRegistry &blockDefinitions,
                                       const ItemDefinitionRegistry &itemDefinitions,
                                       std::shared_ptr<const ProtocolCodec> codec)
        : mBlockDefinitions(blockDefinitions), mItemDefinitions(itemDefinitions), mCodec(std::move(codec)) {}

const ProtocolCodec &PacketCodecContext::getCodec() const {
    return mCodec != nullptr ? *mCodec : ProtocolCodecRegistry::instance().getDefault();
}
