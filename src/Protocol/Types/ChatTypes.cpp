#include "Protocol/Types/ChatTypes.h"

void SignedChat::write(BinaryStream &stream) const {
    stream.putString(mKeyId);
    stream.putString(mSenderId);
    stream.putString(mRoomId);
    stream.putString(mMessageId);
    stream.putString(mScanText);
    stream.putString(mRedactedText);
    stream.putString(mSignature);
}

void SignedChat::read(ReadOnlyBinaryStream &stream) {
    mKeyId = stream.getString();
    mSenderId = stream.getString();
    mRoomId = stream.getString();
    mMessageId = stream.getString();
    mScanText = stream.getString();
    mRedactedText = stream.getString();
    mSignature = stream.getString();
}

void ChatFormatting::write(BinaryStream &stream) const {
    stream.putBool(mHasNamePrefix);
    if (mHasNamePrefix)
        stream.putString(mNamePrefix);
    stream.putBool(mHasNamePostfix);
    if (mHasNamePostfix)
        stream.putString(mNamePostfix);
    stream.putBool(mHasMessagePrefix);
    if (mHasMessagePrefix)
        stream.putString(mMessagePrefix);
    stream.putBool(mHasMessagePostfix);
    if (mHasMessagePostfix)
        stream.putString(mMessagePostfix);
}

void ChatFormatting::read(ReadOnlyBinaryStream &stream) {
    mHasNamePrefix = stream.getBool();
    if (mHasNamePrefix)
        mNamePrefix = stream.getString();
    mHasNamePostfix = stream.getBool();
    if (mHasNamePostfix)
        mNamePostfix = stream.getString();
    mHasMessagePrefix = stream.getBool();
    if (mHasMessagePrefix)
        mMessagePrefix = stream.getString();
    mHasMessagePostfix = stream.getBool();
    if (mHasMessagePostfix)
        mMessagePostfix = stream.getString();
}
