
struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

struct MODEL_CB
{
    float4 colorMultiplier;
};

ConstantBuffer<MODEL_CB> modelCB : register(b0);

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color * modelCB.colorMultiplier;
    return output;
}