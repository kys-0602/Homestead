#pragma once

#include <cstdint>

namespace Homestead {

enum class MapId : std::uint8_t {
    Farm = 0,
    House = 1,
    Count
};

[[nodiscard]] constexpr bool IsValidMapId(MapId id) noexcept {
    return id < MapId::Count;
}

} // namespace Homestead
