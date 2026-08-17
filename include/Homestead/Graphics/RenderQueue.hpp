#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Homestead {

enum class SpriteLayer : std::uint8_t {
    Ground,
    GroundDecoration,
    ObjectBack,
    Actor,
    ObjectFront,
    Effect,
    UI,
    Debug
};

struct SpriteCommand {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
    std::uint16_t uvX = 0;
    std::uint16_t uvY = 0;
    std::uint16_t uvWidth = 0;
    std::uint16_t uvHeight = 0;
    std::uint16_t depth = 0;
    std::uint16_t sortY = 0;
    std::uint16_t textureId = 0;
    std::uint32_t color = 0xFFFFFFFFU;
    SpriteLayer layer = SpriteLayer::Ground;
    std::uint8_t material = 0;
    std::uint16_t submissionOrder = 0;
};

class RenderQueue final {
public:
    static constexpr std::size_t Capacity = 1024;

    void Clear() noexcept;
    [[nodiscard]] bool Add(const SpriteCommand& command) noexcept;
    void Sort() noexcept;

    [[nodiscard]] const SpriteCommand* Data() const noexcept { return commands_.data(); }
    [[nodiscard]] std::size_t Size() const noexcept { return size_; }
    [[nodiscard]] bool Empty() const noexcept { return size_ == 0; }
    [[nodiscard]] const SpriteCommand& operator[](std::size_t index) const noexcept {
        return commands_[index];
    }

private:
    std::array<SpriteCommand, Capacity> commands_{};
    std::size_t size_ = 0;
};

[[nodiscard]] std::size_t FindSpriteBatchEnd(
    const RenderQueue& queue,
    std::size_t begin,
    std::size_t batchCapacity) noexcept;

[[nodiscard]] std::size_t CountSpriteBatches(
    const RenderQueue& queue,
    std::size_t batchCapacity) noexcept;

} // namespace Homestead
