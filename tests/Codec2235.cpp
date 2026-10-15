#include "Protocol/Codec/ProtocolCodecRegistry.h"
#include "Protocol/Packets/ChatPacket.h"
#include "Protocol/Packets/ClientboundAttributeLayerSyncPacket.h"
#include "Protocol/Packets/CommandBlockUpdatePacket.h"
#include "Protocol/Packets/CommandOutputPacket.h"
#include "Protocol/Packets/StructureBlockUpdatePacket.h"
#include "Protocol/Types/StartGameTypes.h"

#include <stdexcept>

static void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}

// Fixtures follow Gophertunnel db9867d field order and primitive encodings.
static std::string unhex(const std::string &hex) {
    std::string bytes;
    for (size_t i = 0; i < hex.size(); i += 2)
        bytes.push_back((char) std::stoul(hex.substr(i, 2), nullptr, 16));
    return bytes;
}

template<class T>
static void fixture(const T &packet, const PacketCodecContext &context, const std::string &hex) {
    const std::string expected = unhex(hex);
    BinaryStream output;
    packet.write(output, context);
    check(output.getBuffer() == expected, "encoding differs from reference fixture");
    ReadOnlyBinaryStream input(expected + std::string(1, char(0xab)));
    T decoded;
    decoded.read(input, context);
    check(input.getByte() == 0xab && input.getRemainingLength() == 0, "incorrect field boundaries");
    BinaryStream roundTrip;
    decoded.write(roundTrip, context);
    check(roundTrip.getBuffer() == expected, "decoded fields differ from reference fixture");
}

int main() {
    BlockDefinitionRegistry blocks;
    ItemDefinitionRegistry items;
    auto &registry = ProtocolCodecRegistry::instance();
    check(registry.getProtocolVersions() == std::vector<int>({2168, 2169, 2193, 2225, 2235}), "missing codecs");
    check(registry.getDefault().getProtocolVersion() == 2235, "incorrect default codec");
    PacketCodecContext current(blocks, items, registry.find(2235));
    PacketCodecContext previous(blocks, items, registry.find(2225));
    check(current.getCodec().supports(MinecraftPacketIds::Chat), "missing chat packet");
    check(!current.getCodec().supports(MinecraftPacketIds::AddVolumeEntity), "obsolete add volume packet");
    check(!current.getCodec().supports(MinecraftPacketIds::RemoveVolumeEntity), "obsolete remove volume packet");
    for (int version: {2168, 2169, 2193, 2225}) {
        const auto codec = registry.find(version);
        check(!codec->supports(MinecraftPacketIds::Chat), "chat exposed to an older protocol");
        check(codec->supports(MinecraftPacketIds::AddVolumeEntity), "legacy add volume packet removed");
        check(codec->supports(MinecraftPacketIds::RemoveVolumeEntity), "legacy remove volume packet removed");
        check(!codec->getCapabilities().mManagedChat && !codec->getCapabilities().mTypedAttributeValues,
              "new fields enabled in an older codec");
    }

    ChatPacket chat;
    chat.mSignedChat.mKeyId = "k";
    chat.mSignedChat.mSenderId = "s";
    chat.mSignedChat.mRoomId = "r";
    chat.mSignedChat.mMessageId = "m";
    chat.mSignedChat.mScanText = "hello";
    chat.mSignedChat.mRedactedText = "safe";
    chat.mSignedChat.mSignature = "sig";
    fixture(chat, current, "016b01730172016d0568656c6c6f04736166650373696700");
    chat.mHasFormatting = true;
    chat.mFormatting.mHasNamePrefix = true;
    chat.mFormatting.mNamePrefix = "[";
    chat.mFormatting.mHasMessagePostfix = true;
    chat.mFormatting.mMessagePostfix = "]";
    fixture(chat, current, "016b01730172016d0568656c6c6f0473616665037369670101015b000001015d");
    BinaryStream withHeader;
    chat.writeWithHeader(withHeader, current);
    check(withHeader.getBuffer().substr(0, 2) == unhex("e902"), "incorrect chat packet ID");

    ServerConfigurationJoinInfo join;
    join.mHasManagedChatJoinInfo = true;
    join.mManagedChatJoinInfo.mEnabled = true;
    BinaryStream joinBytes;
    join.write(joinBytes, true);
    check(joinBytes.getBuffer() == unhex("0000000101"), "incorrect managed chat join info");
    ReadOnlyBinaryStream joinInput(joinBytes.getBuffer());
    ServerConfigurationJoinInfo joined;
    joined.read(joinInput, true);
    check(joined.mHasManagedChatJoinInfo && joined.mManagedChatJoinInfo.mEnabled &&
          joinInput.getRemainingLength() == 0, "incorrect managed chat decoding");
    BinaryStream legacyJoin;
    join.write(legacyJoin, false);
    check(legacyJoin.getBuffer() == unhex("000000"), "legacy join info changed");

    CommandBlockUpdatePacket command;
    command.mLastOutputIsRawMessage = true;
    fixture(command, current, "00000000000000000000000001");
    fixture(command, previous, "000000000000000000000000");

    for (bool raw: {false, true}) {
        CommandOutputPacket output;
        CommandOutputMessage message;
        message.mMessageId = "id";
        message.mInternal = true;
        message.mParameters = {"p"};
        message.mHasRawMessage = raw;
        message.mRawMessage = "{}";
        output.mMessages.push_back(message);
        BinaryStream oldBytes;
        output.write(oldBytes, previous);
        BinaryStream newBytes;
        output.write(newBytes, current);
        const std::string suffix = raw ? unhex("01027b7d") : unhex("00");
        const std::string expected = oldBytes.getBuffer().substr(0, oldBytes.size() - 1) + suffix + std::string(1, char(0));
        check(newBytes.getBuffer() == expected, "incorrect command raw message placement");
        ReadOnlyBinaryStream input(newBytes.getBuffer());
        CommandOutputPacket decoded;
        decoded.read(input, current);
        check(decoded.mMessages[0].mHasRawMessage == raw && input.getRemainingLength() == 0,
              "incorrect command raw message decoding");
    }

    StructureBlockUpdatePacket structure;
    structure.mKeepStructureVoid = true;
    BinaryStream oldStructure;
    structure.write(oldStructure, previous);
    BinaryStream newStructure;
    structure.write(newStructure, current);
    check(newStructure.getBuffer() == oldStructure.getBuffer() + std::string(1, char(1)), "incorrect structure void field");
    ReadOnlyBinaryStream structureInput(newStructure.getBuffer());
    StructureBlockUpdatePacket decodedStructure;
    decodedStructure.read(structureInput, current);
    check(decodedStructure.mKeepStructureVoid && structureInput.getRemainingLength() == 0, "incorrect structure decoding");

    ClientboundAttributeLayerSyncPacket attributes;
    attributes.mData.mType = AttributeLayerSyncPayloadType::UpdateAttributeLayerSettings;
    attributes.mData.mLayerName = "l";
    attributes.mData.mDimension = -1;
    attributes.mData.mSettings.mPriority = 2;
    attributes.mData.mSettings.mWeight.mFloatValue = 0.5f;
    attributes.mData.mSettings.mEnabled = true;
    fixture(attributes, current, "01016c01020000000000003f0100");
    fixture(attributes, previous, "01016c0102000000000000003f0100");
    attributes.mData.mType = AttributeLayerSyncPayloadType::UpdateEnvironmentAttributes;
    EnvironmentAttributeData attribute;
    attribute.mAttributeName = "a";
    attribute.mAttribute.mType = EnvironmentAttributeValueType::String;
    attribute.mAttribute.mStringValue = "hi";
    attributes.mData.mAttributes = {attribute};
    fixture(attributes, current, "02016c01010161000302686900");
    attributes.mData.mAttributes[0].mAttribute.mHasStringOperation = true;
    fixture(attributes, current, "02016c0101016100030268690100000000");
    attribute.mAttribute.mType = EnvironmentAttributeValueType::Bool;
    attribute.mAttribute.mBoolValue = true;
    attributes.mData.mAttributes = {attribute};
    fixture(attributes, current, "02016c0101016100000100");
    attribute.mAttribute.mType = EnvironmentAttributeValueType::Float;
    attribute.mAttribute.mFloatValue = 0.5f;
    attribute.mAttribute.mHasFloatOperation = true;
    attribute.mAttribute.mFloatOperation = 2;
    attributes.mData.mAttributes = {attribute};
    fixture(attributes, current, "02016c0101016100010000003f01020000000000");
    attribute.mAttribute.mType = EnvironmentAttributeValueType::Color;
    attribute.mAttribute.mColorArgb = (int32_t) 0xff112233u;
    attributes.mData.mAttributes = {attribute};
    fixture(attributes, current, "02016c010101610002332211ff00");
}
