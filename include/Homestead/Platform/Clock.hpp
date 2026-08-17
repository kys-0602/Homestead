#pragma once

#include <cstdint>

namespace Homestead {

class Clock final {
public:
    [[nodiscard]] bool Initialize() noexcept;
    [[nodiscard]] double Tick() noexcept;

private:
    std::int64_t frequency_ = 0;
    std::int64_t previousCounter_ = 0;
};

} // namespace Homestead
