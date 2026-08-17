Texture2D sceneTexture : register(t0);
SamplerState pointSampler : register(s0);

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
};

VertexOutput VSMain(uint vertexId : SV_VertexID) {
    const float2 textureCoordinate = float2((vertexId << 1) & 2, vertexId & 2);

    VertexOutput output;
    output.position = float4(
        textureCoordinate.x * 2.0F - 1.0F,
        1.0F - textureCoordinate.y * 2.0F,
        0.0F,
        1.0F);
    output.textureCoordinate = textureCoordinate;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET {
    return sceneTexture.Sample(pointSampler, input.textureCoordinate);
}
