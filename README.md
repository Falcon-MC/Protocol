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
	<img src="https://img.shields.io/badge/minecraft-v1.26.50%20(Bedrock)-56383E" alt="Minecraft">
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
- **NBT** - `Tag` and `NbtIo` with big endian, little endian and network (varint) encodings
- **Utilities** - `BinaryStream` / `ReadOnlyBinaryStream`, a JSON parser, math types and logging

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

## Related repositories

- [Falcon](https://github.com/Falcon-MC/Falcon) - the server
- [Network](https://github.com/Falcon-MC/Network) - RakNet and NetherNet transport

## Licensing information

Falcon is not affiliated with Mojang. All brands and trademarks belong to their respective owners.
