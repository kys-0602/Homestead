#pragma once

#include <cstddef>
#include <cstdint>

namespace Homestead {

class AssetStore;
class Inventory;
class RenderQueue;

inline constexpr std::int32_t InventorySlotSize = 18;
inline constexpr std::int32_t HotbarX = 88;
inline constexpr std::int32_t HotbarY = 160;
inline constexpr std::int32_t InventoryOverlayY = 122;

[[nodiscard]] int InventorySlotAt(std::uint32_t x, std::uint32_t y, bool open) noexcept;
[[nodiscard]] bool AddInventoryUI(
    const Inventory& inventory,
    std::size_t selectedSlot,
    std::size_t cursorSlot,
    bool open,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead
