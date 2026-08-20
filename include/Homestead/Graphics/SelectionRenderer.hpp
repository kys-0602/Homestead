#pragma once

namespace Homestead {

class AssetStore;
class Camera2D;
class RenderQueue;
struct TileSelection;

[[nodiscard]] bool AddSelectionOverlay(
    const TileSelection& selection,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead
