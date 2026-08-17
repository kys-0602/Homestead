#include "Homestead/Graphics/Graphics.hpp"

#if defined(HOMESTEAD_ENABLE_D3D_DEBUG)
#include <d3d11sdklayers.h>
#endif

namespace Homestead {
namespace {

template <typename Interface>
void Release(Interface*& object) noexcept {
    if (object != nullptr) {
        object->Release();
        object = nullptr;
    }
}

constexpr float ClearColor[] = {0.08F, 0.16F, 0.10F, 1.0F};

} // namespace

Graphics::~Graphics() noexcept {
    Shutdown();
}

bool Graphics::Initialize(
    HWND window,
    std::uint32_t clientWidth,
    std::uint32_t clientHeight) noexcept {
    if (device_ != nullptr) {
        return true;
    }

    if (window == nullptr || clientWidth == 0 || clientHeight == 0) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDescription{};
    swapChainDescription.BufferDesc.Width = clientWidth;
    swapChainDescription.BufferDesc.Height = clientHeight;
    swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.SampleDesc.Count = 1;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.BufferCount = 2;
    swapChainDescription.OutputWindow = window;
    swapChainDescription.Windowed = TRUE;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(HOMESTEAD_ENABLE_D3D_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    constexpr D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL createdFeatureLevel{};

    const auto createDevice = [&](D3D_DRIVER_TYPE driverType) noexcept {
        return D3D11CreateDeviceAndSwapChain(
            nullptr,
            driverType,
            nullptr,
            creationFlags,
            featureLevels,
            1,
            D3D11_SDK_VERSION,
            &swapChainDescription,
            &swapChain_,
            &device_,
            &createdFeatureLevel,
            &context_);
    };

    HRESULT result = createDevice(D3D_DRIVER_TYPE_HARDWARE);

#if defined(HOMESTEAD_ENABLE_D3D_DEBUG)
    if (result == DXGI_ERROR_SDK_COMPONENT_MISSING) {
        Shutdown();
        creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        OutputDebugStringW(L"Homestead: D3D11 debug layer is unavailable; continuing without validation.\n");
        result = createDevice(D3D_DRIVER_TYPE_HARDWARE);
    }
#endif

    if (FAILED(result)) {
        Shutdown();
        result = createDevice(D3D_DRIVER_TYPE_WARP);

#if defined(HOMESTEAD_ENABLE_D3D_DEBUG)
        if (result == DXGI_ERROR_SDK_COMPONENT_MISSING) {
            Shutdown();
            creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
            OutputDebugStringW(L"Homestead: D3D11 debug layer is unavailable; continuing without validation.\n");
            result = createDevice(D3D_DRIVER_TYPE_WARP);
        }
#endif
    }

    if (FAILED(result) || createdFeatureLevel != D3D_FEATURE_LEVEL_11_0) {
        Shutdown();
        return false;
    }

    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;
    if (!CreateBackBufferView()) {
        Shutdown();
        return false;
    }

    return true;
}

bool Graphics::Resize(std::uint32_t clientWidth, std::uint32_t clientHeight) noexcept {
    if (swapChain_ == nullptr || clientWidth == 0 || clientHeight == 0) {
        return false;
    }

    if (clientWidth == clientWidth_ && clientHeight == clientHeight_) {
        return true;
    }

    context_->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseBackBufferView();

    const HRESULT result = swapChain_->ResizeBuffers(
        0,
        clientWidth,
        clientHeight,
        DXGI_FORMAT_UNKNOWN,
        0);
    if (FAILED(result)) {
        return false;
    }

    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;
    return CreateBackBufferView();
}

bool Graphics::Render() noexcept {
    if (context_ == nullptr || swapChain_ == nullptr || backBufferView_ == nullptr) {
        return false;
    }

    context_->ClearRenderTargetView(backBufferView_, ClearColor);
    return SUCCEEDED(swapChain_->Present(1, 0));
}

void Graphics::Shutdown() noexcept {
    if (context_ != nullptr) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        context_->ClearState();
        context_->Flush();
    }

    ReleaseBackBufferView();
    Release(swapChain_);
    Release(context_);

#if defined(HOMESTEAD_ENABLE_D3D_DEBUG)
    ID3D11Debug* debug = nullptr;
    if (device_ != nullptr && SUCCEEDED(device_->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug)))) {
        Release(device_);
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        Release(debug);
    } else {
        Release(device_);
    }
#else
    Release(device_);
#endif

    clientWidth_ = 0;
    clientHeight_ = 0;
}

bool Graphics::CreateBackBufferView() noexcept {
    ID3D11Texture2D* backBuffer = nullptr;
    const HRESULT bufferResult = swapChain_->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&backBuffer));
    if (FAILED(bufferResult)) {
        return false;
    }

    const HRESULT viewResult = device_->CreateRenderTargetView(backBuffer, nullptr, &backBufferView_);
    Release(backBuffer);
    return SUCCEEDED(viewResult);
}

void Graphics::ReleaseBackBufferView() noexcept {
    Release(backBufferView_);
}

} // namespace Homestead
