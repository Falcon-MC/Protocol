#pragma once

#include "Core/Utility/BinaryStream.h"
#include "Core/Utility/ReadOnlyBinaryStream.h"

#include <string>

struct SignedChat {
    std::string mKeyId;
    std::string mSenderId;
    std::string mRoomId;
    std::string mMessageId;
    std::string mScanText;
    std::string mRedactedText;
    std::string mSignature;

    void write(BinaryStream &stream) const;
    void read(ReadOnlyBinaryStream &stream);
};

struct ChatFormatting {
    bool mHasNamePrefix = false;
    std::string mNamePrefix;
    bool mHasNamePostfix = false;
    std::string mNamePostfix;
    bool mHasMessagePrefix = false;
    std::string mMessagePrefix;
    bool mHasMessagePostfix = false;
    std::string mMessagePostfix;

    void write(BinaryStream &stream) const;
    void read(ReadOnlyBinaryStream &stream);
};
