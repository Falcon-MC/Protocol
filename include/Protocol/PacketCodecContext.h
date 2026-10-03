#pragma once

#include "Core/Utility/BinaryStream.h"
#include "Core/Utility/ReadOnlyBinaryStream.h"
#include "Protocol/Codec/ProtocolCapabilities.h"
#include "Protocol/Types/BlockDefinitionRegistry.h"
#include "Protocol/Types/ItemDefinitionRegistry.h"

#include <memory>

class ProtocolCodec;

/**
 * What encoding and decoding a packet depends on for one connection: the protocol codec it speaks and the block
 * and item registries its network IDs refer to.
 */
class PacketCodecContext {
public:
    /**
     * A context for the current protocol.
     */
    PacketCodecContext(const BlockDefinitionRegistry &blockDefinitions,
                       const ItemDefinitionRegistry &itemDefinitions);

    PacketCodecContext(const BlockDefinitionRegistry &blockDefinitions,
                       const ItemDefinitionRegistry &itemDefinitions,
                       std::shared_ptr<const ProtocolCodec> codec);

    const BlockDefinitionRegistry &getBlockDefinitions() const { return mBlockDefinitions; }

    const ItemDefinitionRegistry &getItemDefinitions() const { return mItemDefinitions; }

    const ProtocolCodec &getCodec() const;

    const ProtocolCapabilities &getCapabilities() const;

    /**
     * Writes whether an optional field follows, in the form this version expects.
     */
    void putPresence(BinaryStream &stream, bool present) const;

    bool getPresence(ReadOnlyBinaryStream &stream) const;

private:
    const BlockDefinitionRegistry &mBlockDefinitions;
    const ItemDefinitionRegistry &mItemDefinitions;
    std::shared_ptr<const ProtocolCodec> mCodec;
};
