#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;
struct Settings;

inline constexpr std::uint8_t PauseItemCount = 9;

[[nodiscard]] int PauseItemAt(std::uint32_t x, std::uint32_t y) noexcept;
[[nodiscard]] bool AddPauseUI(const Settings& settings, std::uint8_t focus, bool resetConfirmation,
                              const AssetStore& assets, RenderQueue& queue) noexcept;

} // namespace Homestead
