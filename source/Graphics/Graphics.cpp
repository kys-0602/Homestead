#include "Homestead/Graphics/Graphics.hpp"

#include "PresentationPS.hpp"
#include "PresentationVS.hpp"

#include <array>

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

constexpr float SceneClearColor[] = {0.08F, 0.16F, 0.10F, 1.0F};
constexpr float LetterboxColor[] = {0.0F, 0.0F, 0.0F, 1.0F};

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
    presentationViewport_ = CalculatePresentationViewport(clientWidth, clientHeight);
    if (!CreateBackBufferView() ||
        !CreatePresentationResources() ||
        !spriteBatch_.Initialize(device_, context_) ||
        !CreateTestTexture() ||
        !BuildTestRenderQueue()) {
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
    presentationViewport_ = CalculatePresentationViewport(clientWidth, clientHeight);
    return CreateBackBufferView();
}

bool Graphics::Render() noexcept {
    if (context_ == nullptr || swapChain_ == nullptr || backBufferView_ == nullptr ||
        sceneTargetView_ == nullptr || sceneShaderView_ == nullptr || pointSampler_ == nullptr ||
        presentationVertexShader_ == nullptr || presentationPixelShader_ == nullptr ||
        presentationViewport_.width == 0 || presentationViewport_.height == 0) {
        return false;
    }

    D3D11_VIEWPORT sceneViewport{};
    sceneViewport.Width = static_cast<float>(LogicalWidth);
    sceneViewport.Height = static_cast<float>(LogicalHeight);
    sceneViewport.MaxDepth = 1.0F;
    context_->RSSetViewports(1, &sceneViewport);
    context_->OMSetRenderTargets(1, &sceneTargetView_, nullptr);
    context_->ClearRenderTargetView(sceneTargetView_, SceneClearColor);
    if (!spriteBatch_.Render(renderQueue_, testTextureView_, 16, 16, 0) ||
        spriteBatch_.DrawCalls() !=
            CountSpriteBatches(renderQueue_, SpriteBatch::BatchCapacity)) {
        return false;
    }

    D3D11_VIEWPORT outputViewport{};
    outputViewport.TopLeftX = static_cast<float>(presentationViewport_.x);
    outputViewport.TopLeftY = static_cast<float>(presentationViewport_.y);
    outputViewport.Width = static_cast<float>(presentationViewport_.width);
    outputViewport.Height = static_cast<float>(presentationViewport_.height);
    outputViewport.MaxDepth = 1.0F;
    context_->RSSetViewports(1, &outputViewport);
    context_->OMSetRenderTargets(1, &backBufferView_, nullptr);
    context_->ClearRenderTargetView(backBufferView_, LetterboxColor);

    context_->IASetInputLayout(nullptr);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(presentationVertexShader_, nullptr, 0);
    context_->PSSetShader(presentationPixelShader_, nullptr, 0);
    context_->PSSetSamplers(0, 1, &pointSampler_);
    context_->PSSetShaderResources(0, 1, &sceneShaderView_);
    context_->Draw(3, 0);

    ID3D11ShaderResourceView* nullView = nullptr;
    context_->PSSetShaderResources(0, 1, &nullView);

    const HRESULT presentResult = swapChain_->Present(1, 0);
    if (presentResult == DXGI_STATUS_OCCLUDED) {
        return true;
    }

    return SUCCEEDED(presentResult);
}

bool Graphics::ClientToLogical(
    std::int32_t clientX,
    std::int32_t clientY,
    std::uint32_t& logicalX,
    std::uint32_t& logicalY) const noexcept {
    return Homestead::ClientToLogical(
        presentationViewport_,
        clientX,
        clientY,
        logicalX,
        logicalY);
}

void Graphics::Shutdown() noexcept {
    if (context_ != nullptr) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        context_->ClearState();
        context_->Flush();
    }

    ReleaseBackBufferView();
    spriteBatch_.Shutdown();
    ReleaseTestTexture();
    ReleasePresentationResources();
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
    presentationViewport_ = {};
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

bool Graphics::CreatePresentationResources() noexcept {
    D3D11_TEXTURE2D_DESC textureDescription{};
    textureDescription.Width = LogicalWidth;
    textureDescription.Height = LogicalHeight;
    textureDescription.MipLevels = 1;
    textureDescription.ArraySize = 1;
    textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDescription.SampleDesc.Count = 1;
    textureDescription.Usage = D3D11_USAGE_DEFAULT;
    textureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(device_->CreateTexture2D(&textureDescription, nullptr, &sceneTexture_)) ||
        FAILED(device_->CreateRenderTargetView(sceneTexture_, nullptr, &sceneTargetView_)) ||
        FAILED(device_->CreateShaderResourceView(sceneTexture_, nullptr, &sceneShaderView_))) {
        return false;
    }

    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(device_->CreateSamplerState(&samplerDescription, &pointSampler_))) {
        return false;
    }

    if (FAILED(device_->CreateVertexShader(
            HomesteadPresentationVS,
            sizeof(HomesteadPresentationVS),
            nullptr,
            &presentationVertexShader_)) ||
        FAILED(device_->CreatePixelShader(
            HomesteadPresentationPS,
            sizeof(HomesteadPresentationPS),
            nullptr,
            &presentationPixelShader_))) {
        return false;
    }

    return true;
}

bool Graphics::CreateTestTexture() noexcept {
    constexpr std::uint16_t textureWidth = 16;
    constexpr std::uint16_t textureHeight = 16;
    std::array<std::uint32_t, textureWidth * textureHeight> pixels{};
    for (std::uint16_t y = 0; y < textureHeight; ++y) {
        for (std::uint16_t x = 0; x < textureWidth; ++x) {
            const bool border = x < 2 || y < 2 || x >= 14 || y >= 14;
            const bool checker = ((x / 4) + (y / 4)) % 2 == 0;
            const std::uint32_t alpha = border ? 0U : (checker ? 255U : 160U);
            pixels[static_cast<std::size_t>(y) * textureWidth + x] =
                (alpha << 24U) | 0x00FFFFFFU;
        }
    }

    D3D11_TEXTURE2D_DESC description{};
    description.Width = textureWidth;
    description.Height = textureHeight;
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels.data();
    data.SysMemPitch = textureWidth * sizeof(std::uint32_t);
    if (FAILED(device_->CreateTexture2D(&description, &data, &testTexture_)) ||
        FAILED(device_->CreateShaderResourceView(testTexture_, nullptr, &testTextureView_))) {
        return false;
    }

    return true;
}

bool Graphics::BuildTestRenderQueue() noexcept {
    renderQueue_.Clear();

    SpriteCommand command{};
    command.x = 16.0F;
    command.y = 16.0F;
    command.width = 48.0F;
    command.height = 48.0F;
    command.uvWidth = 16;
    command.uvHeight = 16;
    command.layer = SpriteLayer::Ground;
    if (!renderQueue_.Add(command)) {
        return false;
    }

    command.x = 72.0F;
    command.y = 48.0F;
    command.color = 0xC04040FFU;
    command.layer = SpriteLayer::Actor;
    command.sortY = 96;
    if (!renderQueue_.Add(command)) {
        return false;
    }

    command.x = 84.0F;
    command.y = 60.0F;
    command.color = 0xC0FF8040U;
    command.sortY = 108;
    if (!renderQueue_.Add(command)) {
        return false;
    }

    command.x = 250.0F;
    command.y = 20.0F;
    command.width = 32.0F;
    command.height = 32.0F;
    command.color = 0xFF40FFFFU;
    command.layer = SpriteLayer::UI;
    command.sortY = 0;
    if (!renderQueue_.Add(command)) {
        return false;
    }

    renderQueue_.Sort();
    return true;
}

void Graphics::ReleaseBackBufferView() noexcept {
    Release(backBufferView_);
}

void Graphics::ReleasePresentationResources() noexcept {
    Release(presentationPixelShader_);
    Release(presentationVertexShader_);
    Release(pointSampler_);
    Release(sceneShaderView_);
    Release(sceneTargetView_);
    Release(sceneTexture_);
}

void Graphics::ReleaseTestTexture() noexcept {
    Release(testTextureView_);
    Release(testTexture_);
}

} // namespace Homestead
