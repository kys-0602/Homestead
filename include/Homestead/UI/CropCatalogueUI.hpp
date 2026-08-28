#pragma once
namespace Homestead {
class AssetStore; class CropCatalogue; class RenderQueue;
[[nodiscard]] bool AddCropCatalogueUI(const CropCatalogue& catalogue,
    const AssetStore& assets, RenderQueue& queue) noexcept;
} // namespace Homestead
