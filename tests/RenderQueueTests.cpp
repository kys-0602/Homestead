#include "Homestead/Graphics/RenderQueue.hpp"

#include <cstddef>

int main() {
    Homestead::RenderQueue queue;

    Homestead::SpriteCommand command{};
    command.x = 1.0F;
    command.layer = Homestead::SpriteLayer::UI;
    if (!queue.Add(command)) {
        return 1;
    }

    command.x = 2.0F;
    command.layer = Homestead::SpriteLayer::Ground;
    command.depth = 20;
    if (!queue.Add(command)) {
        return 2;
    }

    command.x = 3.0F;
    command.depth = 10;
    if (!queue.Add(command)) {
        return 3;
    }

    command.x = 4.0F;
    command.layer = Homestead::SpriteLayer::Actor;
    command.sortY = 100;
    if (!queue.Add(command)) {
        return 4;
    }

    command.x = 5.0F;
    command.sortY = 40;
    if (!queue.Add(command)) {
        return 5;
    }

    queue.Sort();
    if (queue[0].x != 3.0F || queue[1].x != 2.0F ||
        queue[2].x != 5.0F || queue[3].x != 4.0F || queue[4].x != 1.0F) {
        return 6;
    }

    queue.Clear();
    command = {};
    for (std::size_t index = 0; index < 257; ++index) {
        command.x = static_cast<float>(index);
        if (!queue.Add(command)) {
            return 7;
        }
    }
    queue.Sort();
    if (Homestead::CountSpriteBatches(queue, 256) != 2 ||
        Homestead::FindSpriteBatchEnd(queue, 0, 256) != 256 ||
        Homestead::FindSpriteBatchEnd(queue, 256, 256) != 257) {
        return 8;
    }

    queue.Clear();
    command = {};
    if (!queue.Add(command)) {
        return 9;
    }
    command.textureId = 1;
    if (!queue.Add(command)) {
        return 10;
    }
    queue.Sort();
    if (Homestead::CountSpriteBatches(queue, 256) != 2) {
        return 11;
    }

    queue.Clear();
    command = {};
    for (std::size_t index = 0; index < Homestead::RenderQueue::Capacity; ++index) {
        if (!queue.Add(command)) {
            return 12;
        }
    }
    if (queue.Add(command)) {
        return 13;
    }

    return 0;
}
