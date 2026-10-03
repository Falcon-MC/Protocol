#pragma once

#include "Protocol/Codec/ProtocolCodec.h"

#include <memory>

namespace Codec2193 {

    /**
     * Protocol 2193, Minecraft 1.26.52: every packet keeps its own read and write.
     */
    std::shared_ptr<const ProtocolCodec> create();

}
