#include "Homestead/Graphics/RenderQueue.hpp"

#include <algorithm>

namespace Homestead {
namespace {

bool SpriteLess(const SpriteCommand& left, const SpriteCommand& right) noexcept {
    if (left.textureId != right.textureId) {
        return left.textureId < right.textureId;
    }
    if (left.material != right.material) {
        return left.material < right.material;
    }
    if (left.layer != right.layer) {
        return left.layer < right.layer;
    }

    const std::uint16_t leftSort =
        left.layer == SpriteLayer::Actor ? left.sortY : left.depth;
    const std::uint16_t rightSort =
        right.layer == SpriteLayer::Actor ? right.sortY : right.depth;
    if (leftSort != rightSort) {
        return leftSort < rightSort;
    }

    return left.submissionOrder < right.submissionOrder;
}

} // namespace

void RenderQueue::Clear() noexcept {
    size_ = 0;
}

bool RenderQueue::Add(const SpriteCommand& command) noexcept {
    if (size_ >= commands_.size()) {
        return false;
    }

    commands_[size_] = command;
    commands_[size_].submissionOrder = static_cast<std::uint16_t>(size_);
    ++size_;
    return true;
}

void RenderQueue::Sort() noexcept {
    std::sort(commands_.begin(), commands_.begin() + size_, SpriteLess);
}

std::size_t FindSpriteBatchEnd(
    const RenderQueue& queue,
    std::size_t begin,
    std::size_t batchCapacity) noexcept {
    if (begin >= queue.Size() || batchCapacity == 0) {
        return begin;
    }

    const SpriteCommand& first = queue[begin];
    const std::size_t capacityEnd = std::min(queue.Size(), begin + batchCapacity);
    std::size_t end = begin + 1;
    while (end < capacityEnd &&
           queue[end].textureId == first.textureId &&
           queue[end].material == first.material) {
        ++end;
    }
    return end;
}

std::size_t CountSpriteBatches(
    const RenderQueue& queue,
    std::size_t batchCapacity) noexcept {
    if (batchCapacity == 0) {
        return 0;
    }

    std::size_t count = 0;
    std::size_t begin = 0;
    while (begin < queue.Size()) {
        const std::size_t end = FindSpriteBatchEnd(queue, begin, batchCapacity);
        if (end == begin) {
            return 0;
        }
        begin = end;
        ++count;
    }
    return count;
}

} // namespace Homestead
