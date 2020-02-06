
struct VS_INPUT
{
    ${VSINPUT}
};

struct VS_OUTPUT
{
    ${VSOUTPUT}
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

    ${VSBODY}

    return output;
}