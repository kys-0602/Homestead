#pragma once

#include <d3d11.h>

#include <cstddef>
#include <cstdint>

#include "Homestead/Graphics/RenderQueue.hpp"

namespace Homestead {

class SpriteBatch final {
public:
    static constexpr std::size_t BatchCapacity = 256;

    SpriteBatch() = default;
    ~SpriteBatch() noexcept;

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    [[nodiscard]] bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) noexcept;
    [[nodiscard]] bool Render(
        const RenderQueue& queue,
        ID3D11ShaderResourceView* texture,
        std::uint16_t textureWidth,
        std::uint16_t textureHeight,
        std::uint16_t textureId) noexcept;
    void Shutdown() noexcept;

    [[nodiscard]] std::uint32_t DrawCalls() const noexcept { return drawCalls_; }

private:
    struct SpriteVertex {
        float x;
        float y;
        float u;
        float v;
        std::uint32_t color;
    };

    [[nodiscard]] bool Flush(
        const RenderQueue& queue,
        std::size_t begin,
        std::size_t end,
        std::uint16_t textureWidth,
        std::uint16_t textureHeight) noexcept;

    ID3D11DeviceContext* context_ = nullptr;
    ID3D11Buffer* vertexBuffer_ = nullptr;
    ID3D11Buffer* indexBuffer_ = nullptr;
    ID3D11InputLayout* inputLayout_ = nullptr;
    ID3D11VertexShader* vertexShader_ = nullptr;
    ID3D11PixelShader* pixelShader_ = nullptr;
    ID3D11SamplerState* sampler_ = nullptr;
    ID3D11BlendState* blendState_ = nullptr;
    std::uint32_t drawCalls_ = 0;
};

} // namespace Homestead
