#pragma once

#include "Protocol/Codec/ProtocolCodec.h"

#include <memory>

namespace Codec2225 {

    ProtocolCapabilities capabilities();

    void removeLaterPackets(ProtocolCodec &codec);

    /**
     * Protocol 2225, Minecraft 1.26.60.
     */
    std::shared_ptr<const ProtocolCodec> create();

}
