#include "Homestead/Save/SaveCodec.hpp"

#include <limits>

#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

constexpr std::uint16_t SaveVersion = 1;
constexpr std::uint16_t HeaderSize = 16;

void U8(std::vector<std::uint8_t>& out, std::uint8_t value) { out.push_back(value); }
void U16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    U8(out, static_cast<std::uint8_t>(value)); U8(out, static_cast<std::uint8_t>(value >> 8U));
}
void U32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    U16(out, static_cast<std::uint16_t>(value)); U16(out, static_cast<std::uint16_t>(value >> 16U));
}

std::uint32_t Checksum(const std::uint8_t* data, std::size_t size) noexcept {
    std::uint32_t value = 2166136261U;
    for (std::size_t index = 0; index < size; ++index) {
        value ^= data[index]; value *= 16777619U;
    }
    return value;
}

class Reader final {
public:
    Reader(const std::uint8_t* data, std::size_t size) : data_(data), size_(size) {}
    bool U8(std::uint8_t& value) noexcept {
        if (offset_ >= size_) return false; value = data_[offset_++]; return true;
    }
    bool U16(std::uint16_t& value) noexcept {
        std::uint8_t a = 0, b = 0; if (!U8(a) || !U8(b)) return false;
        value = static_cast<std::uint16_t>(a | static_cast<std::uint16_t>(b << 8U)); return true;
    }
    bool U32(std::uint32_t& value) noexcept {
        std::uint16_t a = 0, b = 0; if (!U16(a) || !U16(b)) return false;
        value = static_cast<std::uint32_t>(a) | (static_cast<std::uint32_t>(b) << 16U); return true;
    }
    std::size_t Remaining() const noexcept { return size_ - offset_; }
private:
    const std::uint8_t* data_ = nullptr; std::size_t size_ = 0; std::size_t offset_ = 0;
};

bool ValidStack(const ItemStack& stack) noexcept {
    if (stack.item == ItemId::None) return stack.count == 0;
    const ItemDefinition* definition = FindItemDefinition(stack.item);
    return definition != nullptr && stack.count != 0 && stack.count <= definition->maximumStack;
}

} // namespace

bool EncodeSave(const SaveSnapshot& snapshot, std::vector<std::uint8_t>& bytes) noexcept {
    if (snapshot.playerX256 < 0 || snapshot.playerY256 < 0 ||
        snapshot.day == 0 || snapshot.minute >= 24U * 60U ||
        snapshot.selectedSlot >= Inventory::HotbarSlotCount ||
        snapshot.harvestedCarrots > 3 || snapshot.tileDeltas.size() > MaximumTileDeltas ||
        snapshot.crops.size() > CropField::Capacity) return false;
    std::vector<std::uint8_t> payload;
    payload.reserve(64 + snapshot.tileDeltas.size() * 3 + snapshot.crops.size() * 4);
    U32(payload, static_cast<std::uint32_t>(snapshot.playerX256));
    U32(payload, static_cast<std::uint32_t>(snapshot.playerY256));
    U16(payload, snapshot.day); U16(payload, snapshot.minute);
    U8(payload, snapshot.selectedSlot); U8(payload, snapshot.harvestedCarrots);
    U16(payload, static_cast<std::uint16_t>(snapshot.tileDeltas.size()));
    U16(payload, static_cast<std::uint16_t>(snapshot.crops.size()));
    for (const ItemStack& stack : snapshot.inventory) {
        if (!ValidStack(stack)) return false;
        U8(payload, static_cast<std::uint8_t>(stack.item)); U8(payload, stack.count);
    }
    for (const SavedTileDelta& delta : snapshot.tileDeltas) {
        U8(payload, delta.x); U8(payload, delta.y); U8(payload, delta.flags);
    }
    for (const CropInstance& crop : snapshot.crops) {
        if (!crop.active || crop.tileX < 0 || crop.tileX > 127 || crop.tileY < 0 || crop.tileY > 127) return false;
        U8(payload, static_cast<std::uint8_t>(crop.tileX)); U8(payload, static_cast<std::uint8_t>(crop.tileY));
        U8(payload, static_cast<std::uint8_t>(crop.crop)); U8(payload, crop.stage);
    }
    if (HeaderSize + payload.size() > MaximumSaveBytes ||
        payload.size() > std::numeric_limits<std::uint32_t>::max()) return false;
    bytes.clear(); bytes.reserve(HeaderSize + payload.size());
    bytes.insert(bytes.end(), {'H', 'S', 'S', 'V'});
    U16(bytes, SaveVersion); U16(bytes, HeaderSize);
    U32(bytes, static_cast<std::uint32_t>(payload.size()));
    U32(bytes, Checksum(payload.data(), payload.size()));
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    return true;
}

bool DecodeSave(const std::uint8_t* bytes, std::size_t size, SaveSnapshot& snapshot) noexcept {
    if (bytes == nullptr || size < HeaderSize || size > MaximumSaveBytes ||
        bytes[0] != 'H' || bytes[1] != 'S' || bytes[2] != 'S' || bytes[3] != 'V') return false;
    Reader header(bytes + 4, HeaderSize - 4);
    std::uint16_t version = 0, headerSize = 0; std::uint32_t payloadSize = 0, checksum = 0;
    if (!header.U16(version) || !header.U16(headerSize) || !header.U32(payloadSize) || !header.U32(checksum) ||
        version != SaveVersion || headerSize != HeaderSize || payloadSize != size - HeaderSize ||
        checksum != Checksum(bytes + HeaderSize, payloadSize)) return false;
    Reader reader(bytes + HeaderSize, payloadSize);
    SaveSnapshot result;
    std::uint32_t x = 0, y = 0; std::uint16_t tileCount = 0, cropCount = 0;
    if (!reader.U32(x) || !reader.U32(y) || !reader.U16(result.day) || !reader.U16(result.minute) ||
        !reader.U8(result.selectedSlot) || !reader.U8(result.harvestedCarrots) ||
        !reader.U16(tileCount) || !reader.U16(cropCount) || result.day == 0 ||
        result.minute >= 24U * 60U || result.selectedSlot >= Inventory::HotbarSlotCount ||
        result.harvestedCarrots > 3 || tileCount > MaximumTileDeltas || cropCount > CropField::Capacity) return false;
    result.playerX256 = static_cast<std::int32_t>(x); result.playerY256 = static_cast<std::int32_t>(y);
    for (ItemStack& stack : result.inventory) {
        std::uint8_t item = 0;
        if (!reader.U8(item) || !reader.U8(stack.count)) return false;
        stack.item = static_cast<ItemId>(item); if (!ValidStack(stack)) return false;
    }
    result.tileDeltas.resize(tileCount);
    for (std::size_t index = 0; index < result.tileDeltas.size(); ++index) {
        SavedTileDelta& delta = result.tileDeltas[index];
        if (!reader.U8(delta.x) || !reader.U8(delta.y) || !reader.U8(delta.flags)) return false;
        const std::uint8_t allowed = TileFlagValue(TileFlag::Tilled) | TileFlagValue(TileFlag::Watered);
        if ((delta.flags & ~allowed) != 0 || (delta.flags & TileFlagValue(TileFlag::Tilled)) == 0 ||
            ((delta.flags & TileFlagValue(TileFlag::Watered)) != 0 &&
             (delta.flags & TileFlagValue(TileFlag::Tilled)) == 0)) return false;
        for (std::size_t other = 0; other < index; ++other)
            if (result.tileDeltas[other].x == delta.x && result.tileDeltas[other].y == delta.y) return false;
    }
    result.crops.resize(cropCount);
    for (std::size_t index = 0; index < result.crops.size(); ++index) {
        CropInstance& crop = result.crops[index];
        std::uint8_t x8 = 0, y8 = 0, id = 0;
        if (!reader.U8(x8) || !reader.U8(y8) || !reader.U8(id) || !reader.U8(crop.stage)) return false;
        crop.tileX = x8; crop.tileY = y8; crop.crop = static_cast<CropId>(id); crop.active = true;
        const CropDefinition* definition = FindCropDefinition(crop.crop);
        if (definition == nullptr || crop.stage > definition->finalStage) return false;
        for (std::size_t other = 0; other < index; ++other)
            if (result.crops[other].tileX == crop.tileX && result.crops[other].tileY == crop.tileY) return false;
    }
    if (reader.Remaining() != 0) return false;
    snapshot = std::move(result); return true;
}

} // namespace Homestead
