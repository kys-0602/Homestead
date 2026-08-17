#include "Homestead/Graphics/SpriteBatch.hpp"

#include "SpritePS.hpp"
#include "SpriteVS.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace Homestead {
namespace {

template <typename Interface>
void Release(Interface*& object) noexcept {
    if (object != nullptr) {
        object->Release();
        object = nullptr;
    }
}

} // namespace

SpriteBatch::~SpriteBatch() noexcept {
    Shutdown();
}

bool SpriteBatch::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) noexcept {
    static_assert(sizeof(SpriteVertex) == 20);

    if (vertexBuffer_ != nullptr) {
        return true;
    }
    if (device == nullptr || context == nullptr) {
        return false;
    }

    context_ = context;

    D3D11_BUFFER_DESC vertexDescription{};
    vertexDescription.ByteWidth =
        static_cast<UINT>(sizeof(SpriteVertex) * BatchCapacity * 4);
    vertexDescription.Usage = D3D11_USAGE_DYNAMIC;
    vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(device->CreateBuffer(&vertexDescription, nullptr, &vertexBuffer_))) {
        Shutdown();
        return false;
    }

    std::array<std::uint16_t, BatchCapacity * 6> indices{};
    for (std::size_t sprite = 0; sprite < BatchCapacity; ++sprite) {
        const std::uint16_t vertex = static_cast<std::uint16_t>(sprite * 4);
        const std::size_t index = sprite * 6;
        indices[index + 0] = vertex;
        indices[index + 1] = static_cast<std::uint16_t>(vertex + 1);
        indices[index + 2] = static_cast<std::uint16_t>(vertex + 2);
        indices[index + 3] = vertex;
        indices[index + 4] = static_cast<std::uint16_t>(vertex + 2);
        indices[index + 5] = static_cast<std::uint16_t>(vertex + 3);
    }

    D3D11_BUFFER_DESC indexDescription{};
    indexDescription.ByteWidth = static_cast<UINT>(sizeof(indices));
    indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
    indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices.data();
    if (FAILED(device->CreateBuffer(&indexDescription, &indexData, &indexBuffer_))) {
        Shutdown();
        return false;
    }

    if (FAILED(device->CreateVertexShader(
            HomesteadSpriteVS,
            sizeof(HomesteadSpriteVS),
            nullptr,
            &vertexShader_)) ||
        FAILED(device->CreatePixelShader(
            HomesteadSpritePS,
            sizeof(HomesteadSpritePS),
            nullptr,
            &pixelShader_))) {
        Shutdown();
        return false;
    }

    constexpr D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    if (FAILED(device->CreateInputLayout(
            inputElements,
            3,
            HomesteadSpriteVS,
            sizeof(HomesteadSpriteVS),
            &inputLayout_))) {
        Shutdown();
        return false;
    }

    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(device->CreateSamplerState(&samplerDescription, &sampler_))) {
        Shutdown();
        return false;
    }

    D3D11_BLEND_DESC blendDescription{};
    D3D11_RENDER_TARGET_BLEND_DESC& targetBlend = blendDescription.RenderTarget[0];
    targetBlend.BlendEnable = TRUE;
    targetBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    targetBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    targetBlend.BlendOp = D3D11_BLEND_OP_ADD;
    targetBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
    targetBlend.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    targetBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    targetBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(device->CreateBlendState(&blendDescription, &blendState_))) {
        Shutdown();
        return false;
    }

    return true;
}

bool SpriteBatch::Render(
    const RenderQueue& queue,
    ID3D11ShaderResourceView* texture,
    std::uint16_t textureWidth,
    std::uint16_t textureHeight,
    std::uint16_t textureId) noexcept {
    if (context_ == nullptr || texture == nullptr || textureWidth == 0 || textureHeight == 0) {
        return false;
    }

    drawCalls_ = 0;
    if (queue.Empty()) {
        return true;
    }

    const UINT stride = sizeof(SpriteVertex);
    constexpr UINT offset = 0;
    context_->IASetInputLayout(inputLayout_);
    context_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
    context_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertexShader_, nullptr, 0);
    context_->PSSetShader(pixelShader_, nullptr, 0);
    context_->PSSetSamplers(0, 1, &sampler_);
    context_->PSSetShaderResources(0, 1, &texture);
    context_->OMSetBlendState(blendState_, nullptr, 0xFFFFFFFFU);

    auto Cleanup = [&]() noexcept {
        ID3D11ShaderResourceView* nullView = nullptr;
        context_->PSSetShaderResources(0, 1, &nullView);
        context_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFFU);
    };

    std::size_t begin = 0;
    while (begin < queue.Size()) {
        const SpriteCommand& command = queue[begin];
        if (command.textureId != textureId || command.material != 0) {
            Cleanup();
            return false;
        }

        const std::size_t end = FindSpriteBatchEnd(queue, begin, BatchCapacity);
        if (end == begin || !Flush(queue, begin, end, textureWidth, textureHeight)) {
            Cleanup();
            return false;
        }
        begin = end;
    }

    Cleanup();
    return true;

void SpriteBatch::Shutdown() noexcept {
    Release(blendState_);
    Release(sampler_);
    Release(pixelShader_);
    Release(vertexShader_);
    Release(inputLayout_);
    Release(indexBuffer_);
    Release(vertexBuffer_);
    context_ = nullptr;
    drawCalls_ = 0;
}

bool SpriteBatch::Flush(
    const RenderQueue& queue,
    std::size_t begin,
    std::size_t end,
    std::uint16_t textureWidth,
    std::uint16_t textureHeight) noexcept {
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(context_->Map(
            vertexBuffer_,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped))) {
        return false;
    }

    auto* vertices = static_cast<SpriteVertex*>(mapped.pData);
    for (std::size_t index = begin; index < end; ++index) {
        const SpriteCommand& command = queue[index];
        const float left = command.x;
        const float top = command.y;
        const float right = command.x + command.width;
        const float bottom = command.y + command.height;
        const float u0 = static_cast<float>(command.uvX) / textureWidth;
        const float v0 = static_cast<float>(command.uvY) / textureHeight;
        const float u1 = static_cast<float>(command.uvX + command.uvWidth) / textureWidth;
        const float v1 = static_cast<float>(command.uvY + command.uvHeight) / textureHeight;
        const std::size_t vertex = (index - begin) * 4;

        vertices[vertex + 0] = {left, top, u0, v0, command.color};
        vertices[vertex + 1] = {right, top, u1, v0, command.color};
        vertices[vertex + 2] = {right, bottom, u1, v1, command.color};
        vertices[vertex + 3] = {left, bottom, u0, v1, command.color};
    }

    context_->Unmap(vertexBuffer_, 0);
    const UINT spriteCount = static_cast<UINT>(end - begin);
    context_->DrawIndexed(spriteCount * 6, 0, 0);
    ++drawCalls_;
    return true;
}

} // namespace Homestead
