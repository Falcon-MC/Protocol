#pragma once

/**
 * What a protocol version supports beyond its packet list. Versions that remove or replace a feature describe
 * it here, so the server can adapt or refuse instead of sending what the client cannot read.
 */
struct ProtocolCapabilities {
    int mProtocolVersion = 0;
};
