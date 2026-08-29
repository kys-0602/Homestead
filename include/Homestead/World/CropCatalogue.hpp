#pragma once

#include <cstdint>

#include "Homestead/World/CropField.hpp"

namespace Homestead {

class CropCatalogue final {
public:
    [[nodiscard]] bool Discover(CropId crop, ItemQuality quality = ItemQuality::Normal) noexcept {
        const std::uint8_t bit = Bit(crop);
        const std::uint16_t qualityBit = QualityBit(crop, quality);
        if (bit == 0 || (quality != ItemQuality::Normal && qualityBit == 0)) return false;
        const bool changed = (discovered_ & bit) == 0;
        discovered_ |= bit;
        const bool qualityChanged = qualityBit != 0 && (qualities_ & qualityBit) == 0;
        qualities_ |= qualityBit;
        return changed || qualityChanged;
    }
    [[nodiscard]] bool IsDiscovered(CropId crop) const noexcept {
        const std::uint8_t bit = Bit(crop);
        return bit != 0 && (discovered_ & bit) != 0;
    }
    [[nodiscard]] bool HasQuality(CropId crop, ItemQuality quality) const noexcept {
        const std::uint16_t bit = QualityBit(crop, quality);
        return bit != 0 && (qualities_ & bit) != 0;
    }
    [[nodiscard]] bool Restore(std::uint8_t discovered, std::uint16_t qualities = 0) noexcept {
        if (!IsValidBits(discovered) || !AreValidQualityBits(qualities)) return false;
        discovered_ = discovered;
        qualities_ = qualities;
        return true;
    }
    [[nodiscard]] std::uint8_t Bits() const noexcept { return discovered_; }
    [[nodiscard]] std::uint16_t QualityBits() const noexcept { return qualities_; }
    void Clear() noexcept { discovered_ = 0; qualities_ = 0; }
    [[nodiscard]] static constexpr bool IsValidBits(std::uint8_t discovered) noexcept {
        return (discovered & ~KnownMask) == 0;
    }
    [[nodiscard]] static constexpr bool AreValidQualityBits(std::uint16_t qualities) noexcept {
        return (qualities & ~KnownQualityMask) == 0;
    }
private:
    [[nodiscard]] static constexpr std::uint8_t Bit(CropId crop) noexcept {
        const std::uint8_t value = static_cast<std::uint8_t>(crop);
        return value == 0 || value > static_cast<std::uint8_t>(CropId::Cabbage) ? 0 :
            static_cast<std::uint8_t>(1U << (value - 1U));
    }
    static constexpr std::uint8_t KnownMask = static_cast<std::uint8_t>(
        (1U << static_cast<std::uint8_t>(CropId::Cabbage)) - 1U);
    [[nodiscard]] static constexpr std::uint16_t QualityBit(CropId crop, ItemQuality quality) noexcept {
        const std::uint8_t cropValue = static_cast<std::uint8_t>(crop);
        if (cropValue == 0 || cropValue > static_cast<std::uint8_t>(CropId::Cabbage)) return 0;
        const std::uint8_t qualityOffset = quality == ItemQuality::Silver ? 0 :
            quality == ItemQuality::Gold ? 1 : 2;
        return qualityOffset == 2 ? 0 : static_cast<std::uint16_t>(
            1U << ((cropValue - 1U) * 2U + qualityOffset));
    }
    static constexpr std::uint16_t KnownQualityMask = static_cast<std::uint16_t>(
        (1U << (static_cast<std::uint8_t>(CropId::Cabbage) * 2U)) - 1U);
    std::uint8_t discovered_ = 0;
    std::uint16_t qualities_ = 0;
};

} // namespace Homestead
