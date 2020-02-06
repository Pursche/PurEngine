
struct PS_INPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // return interpolated color
    //return float4(input.pos.x / 1280.0f, input.pos.y / 720.0f, 0.0f, 1.0f);
    return input.color;
}