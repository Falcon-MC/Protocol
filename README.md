<p align="center">
	<picture>
		<source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/Falcon-MC/Falcon/main/.github/logo-white.png">
		<img src="https://raw.githubusercontent.com/Falcon-MC/Falcon/main/.github/logo.png" alt="Falcon" width="200">
	</picture>
	<br>
	<b>Falcon Protocol</b>
	<br>
	Minecraft: Bedrock Edition protocol library written in C++17
</p>

<p align="center">
	<img src="https://img.shields.io/badge/minecraft-v1.26.51%20(Bedrock)-56383E" alt="Minecraft">
	<img src="https://img.shields.io/badge/protocol-2193-blue" alt="Protocol">
	<img src="https://img.shields.io/badge/language-C%2B%2B17-00599C" alt="C++17">
</p>

## What is this?

The binary format of the Bedrock protocol, with no game logic attached. It is the lowest layer of
[Falcon](https://github.com/Falcon-MC/Falcon) and has no dependency on the server or the transport.

- **Packets** - one class per Bedrock packet in `Protocol/Packets`, each with `write`, `read` and a
  typed `handle` dispatch through `NetworkPacketHandler`
- **Packet ids and factory** - `MinecraftPacketIds` and `MinecraftPackets` create a packet from its id
- **Network types** - items, block definitions, inventory sources, commands, camera, attributes and
  more in `Protocol/Types`
- **Codecs** - items (inventory and entity formats), entity metadata, skins, inventory transactions,
  camera and data store
- **Utilities** - a JSON parser, math types and logging

NBT tags and binary streams come from [NBT](https://github.com/Falcon-MC/NBT), fetched automatically.

## Usage

The library is a plain CMake target named `FalconProtocol`. The easiest way to use it is
`FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    falcon_protocol
    GIT_REPOSITORY https://github.com/Falcon-MC/Protocol.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(falcon_protocol)

target_link_libraries(your_target PRIVATE FalconProtocol)
```

## Building

Only CMake 3.16+ and a C++17 compiler are required.

```
cmake -B build -G Ninja
cmake --build build
```

## Checking the packets

`tools/check_protocol.py` compares every packet's `write` with the official
[protocol documentation](https://github.com/Mojang/bedrock-protocol-docs). It expands both into the encodings that
reach the wire, field by field, and reports packet id differences, missing packets and the first field that differs
in each packet. Every branch a writer can take (`if`/`else`, `switch`, early `return`) must match one of the
documented variants, and helpers called on members or through function arguments are followed. The Protocol check
workflow runs it against the documentation of `CURRENT_PROTOCOL` and publishes the report in the run summary.

```
python tools/check_protocol.py --docs /path/to/bedrock-protocol-docs/json
python tools/check_protocol.py --docs /path/to/bedrock-protocol-docs/json --explain PlaySoundPacket
```

## Related repositories

- [Falcon](https://github.com/Falcon-MC/Falcon) - the server
- [Network](https://github.com/Falcon-MC/Network) - RakNet and NetherNet transport
- [NBT](https://github.com/Falcon-MC/NBT) - NBT tags and binary streams
- [BedrockData](https://github.com/Falcon-MC/BedrockData) - game data files, versioned by protocol
## Licensing information

Falcon Protocol is licensed under the [GNU Lesser General Public License v3.0](LICENSE), which supplements
the [GNU General Public License v3.0](COPYING). It can be linked from projects under any license, as long as
changes to this library itself stay under the same license.

Falcon is not affiliated with Mojang. All brands and trademarks belong to their respective owners.
