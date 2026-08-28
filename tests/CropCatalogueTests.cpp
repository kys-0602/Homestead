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
    catalogue.Clear();
    return catalogue.Bits() == 0 ? 0 : 3;
}
