---
name: godot-port-plan
description: Plan for porting the C++/DX9 Tales of Pirate client to Godot (C#, full parity)
metadata:
  type: project
---

Port of the DX9 client (MindPower3D) to **Godot, language C#, goal — full feature parity**. The servers (.NET Corsairs.* + C++ GameServer) stay as-is; only the client is ported.

**Decisions (locked 2026-06-28):**
- Client language: C# (matches the .NET servers).
- Goal: full parity.
- Protocol: shared contracts assembly — but that is the **destination**, not the first step. Hand-port the minimum first, then extract into a shared library and have the F# servers reference it.
- Security: **server-side packet validation**, not obfuscation. Treat the client as a fully-known, hostile input source. Obfuscation is useless against packet abuse. The only client-side secret worth protecting is the RSA/AES login handshake.

**Status:**
- Servers — done, untouched.
- Assets — solved. Tool `dcx2/PKO-file-viewer2025` (C#) exports `.lgo`/`.lmo`/`.lab` → glTF (meshes, skeletons, animations, textures). User has compatible older clients to run it against. IMPORTANT: that tool needs the vanilla layout `Client\scripts\table\*.bin` (Characterinfo.bin etc.) — in THIS repo those were deleted (migrated into gamedata.sqlite), so convert from an **old** client, not from this repo. Godot reads PNG/DDS natively. Godot prefers glTF (`.glb`) over FBX.

**4 workstreams (order: risky integration first, bulky-but-safe last):**
1. **Network (critical path)** — opcodes from `CmdNames.h` (~383) → C# enum; transport `CorsairsNet` (TCP + WPacket/RPacket + MessagePack) → C#; (de)serialization `PacketCmd_CS.cpp` (CS) + `PacketCmd_SC.cpp` (SC, the big one); RSA/AES handshake.
2. **Login→world flow** — Godot scenes mirroring `Scene/World/`: LoginScene→SelectChaScene→CreateChaScene→WorldScene. First integration milestone (login to the live servers).
3. **Runtime/gameplay (reproduce, don't port)** — state machine from `Scene/Nodes/Character/State/` (Move/Attack/Pose/Die/Seat/NpcTalk) on Godot nodes; `ServerHarm/` (applying authoritative updates); `CharacterModel.cpp` (ID→model+equipment+animation, the bridge to .glb); terrain/scene objects.
4. **UI parity** — 54 forms in `Gui/Forms/` → Godot Control scenes. Logic already specified, only layout is new. Parallelizable, do last.

**Open gaps:**
- Terrain `.map`/`.rbo` heightmaps — the viewer likely does NOT convert these (only map objects). We write this converter part ourselves; reference: `sources/Engine/Map/MapLoader.cpp`.
- Server validation — before relying on the server as the authority, confirm attack/trade/buy actually validate input (`sources/Server/GameServer/src/Combat/FightAble.cpp`).

**Vertical slice (first real target):** transport+handshake → login to live servers → spawn one `.glb` → walk → attack one mob. Exercises assets+protocol+live server together. After that — scaling a proven pattern.

**Architectural fact:** the client is server-authoritative (`ServerHarm/` applies server truth; the `ST*` states mostly *request* and *react*). The port is a presentation re-skin over an unchanged protocol, not a reimplementation of game logic. That's why the hard parts are assets and networking, and gameplay is mostly translation.

**Next step:** scope the network port — read `CmdNames.h`, `PacketCmd_CS/SC`, the transport/handshake → opcode inventory + minimal packet set for login→move→attack + a concrete C# net-layer plan.
