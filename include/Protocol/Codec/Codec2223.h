#pragma once

#include "Protocol/Codec/ProtocolCodec.h"

#include <memory>

namespace Codec2223 {

    /**
     * Protocol 2223, Minecraft 1.26.60: every packet keeps its own read and write.
     */
    std::shared_ptr<const ProtocolCodec> create();

}
