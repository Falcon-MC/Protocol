#include "Protocol/DataStoreCodec.h"

#include "Core/Utility/BinaryDataException.h"

#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

int main() {
    try {
        // a change value of type map (6) nests another change value; a chain deeper than the limit must be
        // rejected with an exception rather than recursing until the stack overflows.
        BinaryStream wire;
        const int depth = 200;
        for (int i = 0; i < depth; ++i) {
            wire.putLInt(6);              // DataStoreChangeValueType::Map
            wire.putUnsignedVarInt(1);    // one entry
            wire.putString("k");          // its key
        }
        wire.putLInt(0);                  // innermost: Null

        ReadOnlyBinaryStream input(wire.getBuffer());
        bool threw = false;
        try {
            DataStoreCodec::readChangeValue(input);
        } catch (const BinaryDataException &) {
            threw = true;
        }
        require(threw, "deeply nested data store map was not rejected");

        // a shallow map still decodes
        BinaryStream shallow;
        shallow.putLInt(6);
        shallow.putUnsignedVarInt(1);
        shallow.putString("k");
        shallow.putLInt(1);               // Bool
        shallow.putBool(true);
        ReadOnlyBinaryStream shallowInput(shallow.getBuffer());
        const DataStoreChangeValue value = DataStoreCodec::readChangeValue(shallowInput);
        require(value.mType == DataStoreChangeValueType::Map, "shallow map lost its type");
        require(value.mMapValue.size() == 1, "shallow map lost its entry");

        std::cout << "DataStore decode tests passed" << std::endl;
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << std::endl;
        return 1;
    }
}
