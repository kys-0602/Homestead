# AGENTS.md

## Purpose

This file defines the working rules for coding agents modifying the Homestead repository. It applies to the repository root and every subdirectory unless a more specific `AGENTS.md` exists below it.

Homestead is a small 2D farming game built with C++17, Win32, and Direct3D 11. The game is inspired by the core interaction loop of Stardew Valley, but it is deliberately much smaller. The submission, including the executable, packaged assets, and a representative save file, must fit within one 1.44MB floppy-disk image:

```text
Homestead.exe + data.pak + representative.sav <= 1,474,560 bytes
```

Binary size is a product requirement, not a final cleanup task.

## Read Before Changing Code

Read the following documents before making architectural or implementation changes:

1. `ARCHITECTURE.md` — ownership, layers, runtime design, and size strategy
2. `IMPLEMENTATION_ROADMAP.md` — implementation order and completion criteria
3. `CMAKE_BUILD_GUIDE.md` — supported local build workflow
4. `Questions.md` — product decisions and the user's original answers
5. `DEVELOPMENT_PROGRESS.md` — completed work, verification history, size measurements, and the next unfinished stage

When the documents conflict, use this priority:

1. The user's latest explicit instruction
2. `AGENTS.md`
3. `ARCHITECTURE.md`
4. `IMPLEMENTATION_ROADMAP.md`
5. Existing implementation

Do not silently resolve a material product conflict. Record the assumption in the response or ask when the choice would lead to substantially different work.

## Current Product Scope

Implement only the following confirmed scope unless the user expands it:

- Windows 10 or newer
- Direct3D feature level 11_0 or newer
- x64 as the initial target
- C++17 and MSVC through CMake
- 16:9 presentation
- 320x180 initial logical resolution
- 16x16 tile and purchased pixel-art assets
- integer scaling, letterboxing, and point sampling
- English-only text rendered with a small bitmap font
- keyboard and mouse input
- smooth player movement with AABB collision
- interaction and tool use on the tile in front of the player or a valid mouse-selected tile
- offline single-player only
- farming, inventory, time/day progression, save/load, minimal goal/completion flow
- background music and sound effects with a strict shared size budget
- `Homestead.exe` plus `data.pak`; development tools are not submitted

Do not pre-implement these excluded systems:

- NPCs, schedules, friendship, or full dialogue systems
- seasons or weather
- multiplayer, networking, or deterministic lockstep infrastructure
- controller/XInput support
- a general-purpose physics engine
- a general-purpose scripting VM
- a general-purpose ECS framework
- a runtime PNG, JSON, or source-HLSL pipeline in the Release game

A small `MessageTrigger` for instructions is allowed. New gameplay such as combat, fishing, mining, crafting, or a shop must be explicitly requested or selected as part of the final content pass.

## Implementation Order

Follow `IMPLEMENTATION_ROADMAP.md`. The next unfinished milestone takes precedence over speculative later systems.

The initial milestone is strictly:

```text
Win32 Application/Window
-> D3D11 device, context, and swap chain
-> clear and present
-> 320x180 scene render target
-> integer-scaled letterbox presentation
```

Do not add tilemaps, farming, inventory, audio, or broad abstraction layers before this milestone works and has been verified. Each subsequent stage must leave the project buildable and runnable.

## Architecture Rules

### Ownership

- `Application` is the composition root and owns long-lived services by value where practical.
- Prefer unique ownership and non-owning references over `shared_ptr`.
- Use RAII for Win32 handles and COM resources.
- A small COM smart pointer is acceptable. Do not introduce a large dependency only for COM ownership.
- Do not add service locators or global mutable singletons.
- Global `constexpr` data is acceptable.
- Member declaration order must respect construction and reverse-destruction dependencies.

### Dependency Direction

Keep dependencies flowing in this direction:

```text
Main/Application
-> Platform, Graphics, Input, Audio, Assets, Save
-> Game/Scene
-> World, Systems, UI, data types
```

- Game rules must not include or expose `HWND`, D3D11 interfaces, or DXGI interfaces.
- Graphics must not mutate gameplay state.
- Platform events must be translated into platform-neutral input/window state.
- Render code consumes a read-only render queue or snapshot.
- Save data must use stable IDs and explicit fields, never pointers or raw runtime object dumps.

### Runtime Model

- Run gameplay at a fixed 60Hz update rate.
- Render at a variable rate and interpolate only when useful.
- Clamp long frame deltas and cap fixed-update iterations.
- Do not consume a `pressed` input more than once when several fixed updates occur in one rendered frame.
- Pause world simulation while allowing UI and input presentation to continue.
- Keep the first implementation single-threaded. Add concurrency only after measuring a real stall.

### Entity and Gameplay Data

- Use small integer handles or IDs for assets, items, crops, maps, and entities.
- If multiple entities become necessary, use fixed-capacity component pools with generation-checked IDs.
- Do not force unique objects such as `PlayerState` and `WorldClock` into an ECS.
- Use data tables plus explicit systems or switches instead of an inheritance class per item/crop type.
- Use dense arrays, sorted arrays, bitsets, and free lists in hot runtime paths.
- Defer entity creation/deletion until the current system iteration ends.

## Direct3D 11 Rules

- Use D3D11 and DXGI from the Windows SDK; do not add the legacy DirectX SDK.
- Enable the D3D11 debug layer only in Debug builds.
- Report or inspect live D3D objects during Debug shutdown.
- Correctly release back-buffer views before resizing swap-chain buffers.
- Skip rendering while the client area is minimized or zero-sized.
- Render gameplay into a 320x180 scene target, then present it with integer scaling and letterboxing.
- Use point sampling for pixel art.
- Snap the final camera position to logical pixels to avoid shimmer.
- Convert mouse coordinates through the letterbox viewport; reject clicks outside it.
- Use one or very few texture atlases and batch sprites through one dynamic vertex buffer.
- Preserve transparent draw order. Use foot-position Y as the actor/object sorting key.
- Compile HLSL offline. Release builds must store bytecode in `data.pak` or a compact embedded form.
- Remove the `d3dcompiler` runtime link once no runtime compilation path requires it.
- Treat device-lost recovery as out of scope unless explicitly requested; fail safely and report the error.

## Asset Rules

- Source assets belong under `assets-src/` and are never part of the submission.
- HLSL source belongs under `shaders/` and is never part of the submission.
- Runtime assets belong in `data.pak`.
- Asset conversion belongs in a separate `AssetPacker` build target under `tools/`.
- The runtime must not decode source PNG, JSON, CSV, WAV, or HLSL unless a measured exception is approved.
- Convert names to compact `AssetId` values and detect collisions in the packer.
- Validate pak magic, version, ranges, offsets, and sizes before use.
- Make pak output deterministic for identical inputs.
- Include only selected assets actually referenced by the game.
- Deduplicate identical tiles, trim unused transparent regions, reduce palettes, and compare compression by total decoder-plus-data cost.
- Keep the English bitmap font to actually used ASCII glyphs where possible.
- Check redistribution rights for purchased art, music, and sound effects before submission.

The purchased pixel-art archive is approximately 5.6MB compressed, so it cannot be shipped wholesale. Do not copy the complete asset pack into runtime data.

## Save Rules

- Save under `%LOCALAPPDATA%/Homestead`, not beside the executable.
- Use an explicit versioned binary format with magic, payload size, and checksum.
- Save player state, inventory, time, changed map tiles/objects, and crops.
- Reference static definitions by stable ID rather than copying them into the save.
- Store only map deltas from packaged source maps.
- Write to a temporary file, close/flush it, then replace the primary save.
- Preserve one previous valid save as a backup.
- Validate all lengths, IDs, enum values, and counts before applying loaded data.
- Keep a representative long-play save at or below 32KiB.
- Do not serialize raw structs when padding, pointer width, or compiler layout can affect the format.

## Binary-Size Policy

Use the following provisional budget:

| Area | Budget |
|---|---:|
| PE executable and game code | 340KiB |
| Shader bytecode | 16KiB |
| Sprite/font atlas | 430KiB |
| Maps, definitions, and strings | 180KiB |
| Music and sound effects | 330KiB |
| Pak index and alignment | 48KiB |
| Representative save | 32KiB |
| Safety margin | 64KiB |
| Total | 1,440KiB |

For every feature that materially changes Release output:

1. Record `Homestead.exe` size in bytes.
2. Record `data.pak` size in bytes when it exists.
3. Record representative save size when saving exists.
4. Report the total and remaining bytes.
5. Investigate a material increase in the stage where it appears.

Prefer smaller direct implementations when measured output supports them. Avoid adding heavy standard-library facilities or third-party libraries casually, especially `iostream`, locale, regex, generic serialization, runtime reflection, and broad framework code. They are not absolutely forbidden, but their Release cost must be measured and justified.

Release builds should use size-oriented optimization, LTCG, dead-code elimination, and identical COMDAT folding. Do not rely on UPX or another executable packer to hide uncontrolled growth. If a packer is used for the final submission, also verify unpacked operation, startup, and antivirus behavior.

## C++ Style

- Use C++17.
- Compile cleanly at MSVC `/W4`; do not suppress a warning globally to avoid fixing local code.
- Use `#pragma once` consistently for project headers.
- Place project code in the `Homestead` namespace.
- Use `PascalCase` for types and functions, `camelCase` for local variables and parameters, and a trailing underscore for private data members.
- Use fixed-width integers at binary, GPU, pak, and save-format boundaries.
- Mark important return values `[[nodiscard]]` where ignoring failure is unsafe.
- Mark non-throwing cleanup and simple accessors `noexcept` when true.
- Prefer `enum class` over unscoped enums.
- Prefer value types and `std::span`-like views where available without adding a large dependency.
- Avoid macro utilities except platform/build configuration and compact assertions.
- Avoid `std::function` and virtual dispatch in per-sprite or per-entity hot loops.
- Low-frequency scene-level virtual dispatch is acceptable.
- Do not use exceptions for ordinary runtime failures. Propagate initialization/load failures with a small result or error-code type.
- Do not use RTTI unless a measured, explicit need appears.
- Do not allocate once per sprite, tile, event, or entity update.
- Keep headers narrow. Do not expose Windows or D3D headers from gameplay-facing interfaces.
- Include what a file uses and use forward declarations where they are safe.
- Comments should explain constraints, ownership, binary layouts, and non-obvious decisions rather than restating code.

If existing code establishes a different local naming convention before this guide is applied, preserve consistency within that subsystem and avoid mechanical renaming unrelated to the task.

## Repository Layout

Use this target structure as files are introduced:

```text
include/Homestead/
  App/
  Platform/
  Graphics/
  Input/
  Audio/
  Assets/
  Core/
  Game/
  World/
  Systems/
  UI/
  Save/
source/
  (mirrors include/Homestead where useful)
assets-src/
shaders/
tools/AssetPacker/
tests/
```

Do not create directories full of unused placeholder classes. Introduce a directory or file when its roadmap stage begins. Small value types may share a focused header; do not create one file per trivial struct merely to match the diagram.

## Build Commands

Use the repository presets from PowerShell:

```powershell
cmake --preset debug
cmake --build --preset debug
```

The Debug executable is expected at:

```text
build/debug/Debug/Homestead.exe
```

Build Release with:

```powershell
cmake --preset release
cmake --build --preset release
```

The Release executable is expected at:

```text
build/release/Release/Homestead.exe
```

The presets currently use the `Visual Studio 18 2026` generator. Do not silently change the generator or minimum CMake version solely to accommodate the agent's machine. If the configured toolchain is unavailable, report that verification could not run or make a narrowly justified compatibility change when the user asks.

Do not delete or recursively recreate the `build/` directory without need. Prefer CMake reconfiguration. Never commit generated build output.

## Verification Requirements

After a code change, run the narrowest relevant checks and then the normal Debug build when available.

At minimum:

- CMake configure when build definitions changed
- Debug build for all C++ changes
- relevant unit tests for pure math, serialization, pak parsing, inventory, or farming rules
- Release build and byte-size report for changes that can affect shipped code or assets
- `git diff --check` for text changes

For D3D11 work, also verify when possible:

- no debug-layer errors
- no live-object report at shutdown
- resize and minimize behavior
- repeated Alt+Tab/focus changes
- multiple window sizes and letterbox math

For save/pak parsers, include malformed-data tests. Test bad magic, unsupported version, truncated data, integer overflow, invalid offset/range, invalid ID, and excessive count. Never trust a length from disk before checking it against the containing buffer and configured maximum.

If GUI execution cannot be observed in the current environment, still build and test all non-visual logic, then clearly state which visual checks remain manual.

## Change Discipline

- Inspect `git status` before editing.
- Preserve user changes and unrelated dirty files.
- The old `Application`, `Window`, `Singleton`, and `Main` files were intentionally deleted. Do not restore them from Git. Create the new architecture only when its roadmap step is requested.
- Do not use destructive Git commands or erase generated/user data without explicit authorization.
- Keep a change focused on the requested roadmap stage.
- Do not refactor unrelated code while implementing a feature.
- Update `ARCHITECTURE.md` when a lasting architectural decision changes.
- Update `IMPLEMENTATION_ROADMAP.md` only when scope, order, or completion criteria change—not merely to mark informal progress.
- Add a more specific nested `AGENTS.md` only when a subsystem genuinely needs different rules.
- After completing and verifying a material task, ask the user whether they want the changes committed and pushed if they have not already given that instruction. Do not commit or push without the user's explicit request.

## Progress Recording

Update `DEVELOPMENT_PROGRESS.md` whenever a roadmap-stage task or other material implementation task is completed. Do this as part of the same change rather than leaving the record for a later cleanup pass.

Record at minimum:

1. What was implemented and the resulting stage status.
2. Automated, build, and GUI verification performed.
3. Any manual or environment-dependent verification still pending.
4. Release sizes for `Homestead.exe`, `data.pak`, and the representative save when each exists.
5. Size change from the previous recorded state, total submitted size, and bytes remaining under the limit.
6. The next unfinished roadmap stage or concrete follow-up task.

After the work is merged, update the record with the canonical `main` commit or PR number at the next opportunity. Do not use `IMPLEMENTATION_ROADMAP.md` as a progress checklist; it remains the source for scope, order, and completion criteria.

## Definition of Done

A roadmap stage is complete only when:

- the project configures and builds in the relevant configuration;
- the stage's completion criteria in `IMPLEMENTATION_ROADMAP.md` are satisfied;
- ownership and dependency direction remain consistent with `ARCHITECTURE.md`;
- failure paths clean up partial initialization safely;
- relevant automated tests pass;
- required manual D3D11 checks are performed or explicitly listed as pending;
- Release size is measured when the change affects shipped output;
- no unrelated user changes were overwritten;
- the final handoff names changed files, verification performed, size impact, and any remaining risk.

Do not describe placeholder code, an untested build, or an empty interface as a completed stage.
