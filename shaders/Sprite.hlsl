Texture2D spriteTexture : register(t0);
SamplerState pointSampler : register(s0);

struct VertexInput {
    float2 position : POSITION;
    float2 textureCoordinate : TEXCOORD0;
    float4 color : COLOR0;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
    float4 color : COLOR0;
};

VertexOutput VSMain(VertexInput input) {
    VertexOutput output;
    output.position = float4(
        input.position.x / 320.0F * 2.0F - 1.0F,
        1.0F - input.position.y / 180.0F * 2.0F,
        0.0F,
        1.0F);
    output.textureCoordinate = input.textureCoordinate;
    output.color = input.color;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET {
    return spriteTexture.Sample(pointSampler, input.textureCoordinate) * input.color;
}
