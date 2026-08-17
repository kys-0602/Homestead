#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <cstdint>

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
    void Shutdown() noexcept;

private:
    [[nodiscard]] bool CreateBackBufferView() noexcept;
    void ReleaseBackBufferView() noexcept;

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapChain_ = nullptr;
    ID3D11RenderTargetView* backBufferView_ = nullptr;
    std::uint32_t clientWidth_ = 0;
    std::uint32_t clientHeight_ = 0;
};

} // namespace Homestead
