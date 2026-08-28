#pragma once

#include <cstdint>

#include "Homestead/World/CropField.hpp"

namespace Homestead {

class CropCatalogue final {
public:
    [[nodiscard]] bool Discover(CropId crop) noexcept {
        const std::uint8_t bit = Bit(crop);
        if (bit == 0) return false;
        const bool changed = (discovered_ & bit) == 0;
        discovered_ |= bit;
        return changed;
    }
    [[nodiscard]] bool IsDiscovered(CropId crop) const noexcept {
        const std::uint8_t bit = Bit(crop);
        return bit != 0 && (discovered_ & bit) != 0;
    }
    [[nodiscard]] bool Restore(std::uint8_t discovered) noexcept {
        if (!IsValidBits(discovered)) return false;
        discovered_ = discovered;
        return true;
    }
    [[nodiscard]] std::uint8_t Bits() const noexcept { return discovered_; }
    void Clear() noexcept { discovered_ = 0; }
    [[nodiscard]] static constexpr bool IsValidBits(std::uint8_t discovered) noexcept {
        return (discovered & ~KnownMask) == 0;
    }
private:
    [[nodiscard]] static constexpr std::uint8_t Bit(CropId crop) noexcept {
        const std::uint8_t value = static_cast<std::uint8_t>(crop);
        return value == 0 || value > static_cast<std::uint8_t>(CropId::Cabbage) ? 0 :
            static_cast<std::uint8_t>(1U << (value - 1U));
    }
    static constexpr std::uint8_t KnownMask = static_cast<std::uint8_t>(
        (1U << static_cast<std::uint8_t>(CropId::Cabbage)) - 1U);
    std::uint8_t discovered_ = 0;
};

} // namespace Homestead
