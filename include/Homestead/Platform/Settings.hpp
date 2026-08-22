#pragma once

#include <cstdint>

namespace Homestead {

struct Settings final {
    std::uint8_t windowScale = 4;
    bool fullscreen = false;
    std::uint8_t masterVolume = 10;
    std::uint8_t musicVolume = 8;
    std::uint8_t effectVolume = 10;
};

[[nodiscard]] bool IsValidSettings(const Settings& settings) noexcept;

class SettingsSystem final {
public:
    [[nodiscard]] bool Load(Settings& settings) const noexcept;
    [[nodiscard]] bool Save(const Settings& settings) const noexcept;
};

} // namespace Homestead
