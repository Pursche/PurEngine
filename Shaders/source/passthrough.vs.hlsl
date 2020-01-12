
struct VS_INPUT
{
    uint vertexIndex : SV_VERTEXID;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.texCoord = float2(input.vertexIndex & 1, input.vertexIndex >> 1);
    output.pos = float4((output.texCoord.x - 0.5f) * 2, -(output.texCoord.y - 0.5f) * 2, 0, 1);

    return output;
}