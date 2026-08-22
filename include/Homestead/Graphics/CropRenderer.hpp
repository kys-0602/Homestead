#pragma once

namespace Homestead {

class AssetStore;
class Camera2D;
class CropField;
class RenderQueue;

[[nodiscard]] bool AddCrops(
    const CropField& field,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead
