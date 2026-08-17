#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <cstdint>

#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Graphics/SpriteBatch.hpp"

namespace Homestead {

class Graphics final {
public:
    Graphics() = default;
    ~Graphics() noexcept;

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    [[nodiscard]] bool Initialize(
        HWND window,
        std::uint32_t clientWidth,
        std::uint32_t clientHeight) noexcept;
    [[nodiscard]] bool Resize(std::uint32_t clientWidth, std::uint32_t clientHeight) noexcept;
    [[nodiscard]] bool Render() noexcept;
    [[nodiscard]] bool ClientToLogical(
        std::int32_t clientX,
        std::int32_t clientY,
        std::uint32_t& logicalX,
        std::uint32_t& logicalY) const noexcept;
    void Shutdown() noexcept;

private:
    [[nodiscard]] bool CreateBackBufferView() noexcept;
    [[nodiscard]] bool CreatePresentationResources() noexcept;
    [[nodiscard]] bool CreateTestTexture() noexcept;
    [[nodiscard]] bool BuildTestRenderQueue() noexcept;
    void ReleaseBackBufferView() noexcept;
    void ReleasePresentationResources() noexcept;
    void ReleaseTestTexture() noexcept;

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapChain_ = nullptr;
    ID3D11RenderTargetView* backBufferView_ = nullptr;
    ID3D11Texture2D* sceneTexture_ = nullptr;
    ID3D11RenderTargetView* sceneTargetView_ = nullptr;
    ID3D11ShaderResourceView* sceneShaderView_ = nullptr;
    ID3D11SamplerState* pointSampler_ = nullptr;
    ID3D11VertexShader* presentationVertexShader_ = nullptr;
    ID3D11PixelShader* presentationPixelShader_ = nullptr;
    ID3D11Texture2D* testTexture_ = nullptr;
    ID3D11ShaderResourceView* testTextureView_ = nullptr;
    SpriteBatch spriteBatch_;
    RenderQueue renderQueue_;
    PresentationViewport presentationViewport_{};
    std::uint32_t clientWidth_ = 0;
    std::uint32_t clientHeight_ = 0;
};

} // namespace Homestead
