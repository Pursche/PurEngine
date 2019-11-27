
struct VS_INPUT
{
    float3 pos : POSITION;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

struct VIEW_CB
{
    float4x4 view;
    float4x4 proj;
};

struct MODEL_CB
{
    float4 color;
    float4x4 model;
};

ConstantBuffer<VIEW_CB> viewCB : register(b0);
ConstantBuffer<MODEL_CB> modelCB : register(b1);

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.pos = mul(mul(mul(float4(input.pos, 1.0f), modelCB.model), viewCB.view), viewCB.proj);
    output.color = modelCB.color;
    return output;
}