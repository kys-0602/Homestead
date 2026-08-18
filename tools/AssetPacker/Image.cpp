#include "Image.hpp"

#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <limits>

namespace Homestead::AssetPacker {
namespace {

Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
bool comInitialized = false;

} // namespace

bool InitializeImageDecoder(std::string& error) {
    const HRESULT initializeResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(initializeResult)) {
        error = "CoInitializeEx failed";
        return false;
    }
    comInitialized = true;
    const HRESULT factoryResult = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
    if (FAILED(factoryResult)) {
        error = "cannot create WIC imaging factory";
        ShutdownImageDecoder();
        return false;
    }
    return true;
}

void ShutdownImageDecoder() noexcept {
    factory.Reset();
    if (comInitialized) {
        CoUninitialize();
        comInitialized = false;
    }
}

bool LoadPng(const std::filesystem::path& path, Image& image, std::string& error) {
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT result = factory->CreateDecoderFromFilename(
        path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(result)) {
        error = "cannot decode " + path.string();
        return false;
    }
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    result = decoder->GetFrame(0, &frame);
    if (FAILED(result)) {
        error = "cannot read PNG frame " + path.string();
        return false;
    }
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    result = factory->CreateFormatConverter(&converter);
    if (FAILED(result) || FAILED(converter->Initialize(
            frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
            nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        error = "cannot convert PNG to RGBA " + path.string();
        return false;
    }
    UINT width = 0;
    UINT height = 0;
    if (FAILED(converter->GetSize(&width, &height)) || width == 0 || height == 0) {
        error = "invalid PNG dimensions " + path.string();
        return false;
    }
    const std::uint64_t byteCount = static_cast<std::uint64_t>(width) * height * 4U;
    if (byteCount > std::numeric_limits<UINT>::max()) {
        error = "PNG is too large " + path.string();
        return false;
    }
    image.width = width;
    image.height = height;
    image.pixels.resize(static_cast<std::size_t>(byteCount));
    result = converter->CopyPixels(
        nullptr, width * 4U, static_cast<UINT>(byteCount), image.pixels.data());
    if (FAILED(result)) {
        error = "cannot copy PNG pixels " + path.string();
        return false;
    }
    return true;
}

} // namespace Homestead::AssetPacker
