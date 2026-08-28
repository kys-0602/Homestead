#pragma once

#include "Homestead/Save/SaveCodec.hpp"

namespace Homestead {

enum class SaveLoadResult {
    NotFound,
    LoadedPrimary,
    LoadedBackup,
    Invalid
};

class SaveSystem final {
public:
    [[nodiscard]] SaveLoadResult Load(SaveSnapshot& snapshot) noexcept;
    [[nodiscard]] bool Save(const SaveSnapshot& snapshot) noexcept;
    [[nodiscard]] bool Reset() noexcept;
};

} // namespace Homestead
