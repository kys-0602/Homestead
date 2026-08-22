#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Input/Action.hpp"

namespace Homestead {

class Input final {
public:
    void BeginFrame() noexcept;
    void SetPhysicalKey(PhysicalKey key, bool held) noexcept;
    void OnFocusLost() noexcept;

    [[nodiscard]] bool Held(Action action) const noexcept;
    [[nodiscard]] bool Pressed(Action action) const noexcept;
    [[nodiscard]] bool Released(Action action) const noexcept;
    [[nodiscard]] bool ConsumePressed(Action action) noexcept;
    [[nodiscard]] bool ConsumePressed(Action action, PhysicalKey& source) noexcept;
    void DiscardPending() noexcept;

    void SetClientMouse(std::int32_t x, std::int32_t y) noexcept;
    void SetLogicalMouse(std::uint32_t x, std::uint32_t y, bool valid) noexcept;

    [[nodiscard]] std::int32_t ClientMouseX() const noexcept { return clientMouseX_; }
    [[nodiscard]] std::int32_t ClientMouseY() const noexcept { return clientMouseY_; }
    [[nodiscard]] std::uint32_t LogicalMouseX() const noexcept { return logicalMouseX_; }
    [[nodiscard]] std::uint32_t LogicalMouseY() const noexcept { return logicalMouseY_; }
    [[nodiscard]] bool IsLogicalMouseValid() const noexcept { return logicalMouseValid_; }

private:
    static constexpr std::size_t ActionCount = static_cast<std::size_t>(Action::Count);
    static constexpr std::size_t KeyCount = static_cast<std::size_t>(PhysicalKey::Count);

    [[nodiscard]] static Action MapAction(PhysicalKey key) noexcept;
    [[nodiscard]] static std::size_t ToIndex(Action action) noexcept;
    [[nodiscard]] static std::size_t ToIndex(PhysicalKey key) noexcept;

    std::array<bool, KeyCount> keyHeld_{};
    std::array<bool, ActionCount> pressed_{};
    std::array<bool, ActionCount> released_{};
    std::array<bool, ActionCount> pendingPressed_{};
    std::array<PhysicalKey, ActionCount> pendingSource_{};
    std::int32_t clientMouseX_ = -1;
    std::int32_t clientMouseY_ = -1;
    std::uint32_t logicalMouseX_ = 0;
    std::uint32_t logicalMouseY_ = 0;
    bool logicalMouseValid_ = false;
};

} // namespace Homestead
