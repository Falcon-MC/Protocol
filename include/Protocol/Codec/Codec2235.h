#pragma once

#include "Protocol/Codec/ProtocolCodec.h"

#include <memory>

namespace Codec2235 {

    /**
     * Protocol 2235, Minecraft 1.26.70: every packet keeps its own read and write.
     */
    std::shared_ptr<const ProtocolCodec> create();

}
