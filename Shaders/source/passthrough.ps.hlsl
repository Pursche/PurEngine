
Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return inputTexture.Sample(inputSampler, input.texCoord);
}