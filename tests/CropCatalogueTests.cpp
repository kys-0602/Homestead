#include "Homestead/World/CropCatalogue.hpp"

int main() {
    Homestead::CropCatalogue catalogue;
    if (catalogue.Bits() != 0 || catalogue.Discover(Homestead::CropId::None) ||
        !catalogue.Discover(Homestead::CropId::Wheat) ||
        catalogue.Discover(Homestead::CropId::Wheat) ||
        !catalogue.Discover(Homestead::CropId::Cabbage) ||
        !catalogue.IsDiscovered(Homestead::CropId::Wheat) ||
        catalogue.IsDiscovered(Homestead::CropId::Carrot)) return 1;
    if (!catalogue.Restore(0x15) || catalogue.Bits() != 0x15 ||
        catalogue.Restore(0x80)) return 2;
    if (!catalogue.Discover(Homestead::CropId::Wheat, Homestead::ItemQuality::Silver) ||
        catalogue.Discover(Homestead::CropId::Wheat, Homestead::ItemQuality::Silver) ||
        !catalogue.HasQuality(Homestead::CropId::Wheat, Homestead::ItemQuality::Silver) ||
        catalogue.HasQuality(Homestead::CropId::Wheat, Homestead::ItemQuality::Gold) ||
        !catalogue.Discover(Homestead::CropId::Wheat, Homestead::ItemQuality::Gold) ||
        !catalogue.HasQuality(Homestead::CropId::Wheat, Homestead::ItemQuality::Gold) ||
        catalogue.QualityBits() != 0x0003 ||
        !catalogue.Restore(0x01, 0x0003) ||
        catalogue.Restore(0x01, 0x1000)) return 3;
    catalogue.Clear();
    return catalogue.Bits() == 0 && catalogue.QualityBits() == 0 ? 0 : 4;
}
