#pragma once

#include "Protocol/Codec/ProtocolCodec.h"

#include <memory>

namespace Codec2169 {

    /**
     * Protocol 2169, Minecraft 1.26.45: the current packets with the format differences described in its
     * capabilities, without the packets added in 2193.
     */
    std::shared_ptr<const ProtocolCodec> create();

}
