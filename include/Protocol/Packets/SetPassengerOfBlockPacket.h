#pragma once

#include "Core/Math/Vector3f.h"
#include "Core/Math/Vector3i.h"
#include "Protocol/Packet.h"

#include <cstdint>

class SetPassengerOfBlockPacket : public Packet {
public:
    static const MinecraftPacketIds ID = MinecraftPacketIds::SetPassengerOfBlock;

    enum class EmoteType : uint8_t {
        Standing = 0,
        Riding = 1,
        Laying = 2
    };

    SetPassengerOfBlockPacket();

    MinecraftPacketIds getId() const override { return ID; }

    const char *getName() const override { return "SetPassengerOfBlockPacket"; }

    void write(BinaryStream &stream, const PacketCodecContext &context) const override;

    void read(ReadOnlyBinaryStream &stream, const PacketCodecContext &context) override;

    void handle(const NetworkIdentifier &id, NetworkPacketHandler &handler) const override;

    int64_t mPassengerUniqueId = 0;
    bool mHasData = false;
    Vector3i mBlockPosition;
    Vector3f mOffset;
    float mRotation = 0.0f;
    float mRotationLimit = 0.0f;
    EmoteType mEmoteType = EmoteType::Standing;
};
