@0xdd5ea4f8672425ee;

enum FilterMode
{
    point @0;
    linear @1;
    anisotropic @2;
}

enum WrapMode
{
    clamp @0;
    wrap @1;
    mirror @2;
    border @3;
}

struct Sampler
{
    name @0 : Text;
    filterXY @1 : FilterMode;
    filterZ @2 : FilterMode;
    wrapModeXY @3 : WrapMode;
    wrapModeZ @4 : WrapMode;
}

enum BlendMode
{
    zero @0;
    one @1;
    srcColor @2;
    invSrcColor @3;
    srcAlpha @4;
    invSrcAlpha @5;
    destColor @6;
    invDestColor @7;
    destAlpha @8;
    invDestAlpha @9;
    blendFactor @10;
    invBlendFactor @11;
}

enum BlendOp
{
    add @0;
    subtract @1;
    revSubtract @2;
    min @3;
    max @4;
}

struct Blender
{
    name @0 : Text;
    enabled @1 : Bool;
    logicOpEnabled @2 : Bool;
    srcBlendMode @3 : BlendMode;
    destBlendMode @4 : BlendMode;
    blendOp @5 : BlendOp;
    srcAlphaBlendMode @6 : BlendMode;
    destAlphaBlendMode @7 : BlendMode;
    alphaBlendOp @8 : BlendOp;
}

enum ParameterType
{
    texture1D @0;
    texture2D @1;
    texture3D @2;
    float @3;
    float2 @4;
    float3 @5;
    float4 @6;
    int @7;
    int2 @8;
    int3 @9;
    int4 @10;
    uint @11;
    uint2 @12;
    uint3 @13;
    uint4 @14;
    bool @15;
    bool2 @16;
    bool3 @17;
    bool4 @18;
    double @19;
    double2 @20;
    double3 @21;
    double4 @22;
    half @23;
    half2 @24;
    half3 @25;
    half4 @26;

    float1x1 @27;
    float1x2 @28;
    float1x3 @29;
    float1x4 @30;
    float2x1 @31;
    float2x2 @32;
    float2x3 @33;
    float2x4 @34;
    float3x1 @35;
    float3x2 @36;
    float3x3 @37;
    float3x4 @38;
    float4x1 @39;
    float4x2 @40;
    float4x3 @41;
    float4x4 @42;

    color @43;
}

enum SubType
{
    float @0;
    float2 @1;
    float3 @2;
    float4 @3;
    int @4;
    int2 @5;
    int3 @6;
    int4 @7;
    uint @8;
    uint2 @9;
    uint3 @10;
    uint4 @11;
}

struct Parameter
{
    name @0 : Text;
    type @1 : ParameterType;
    subType @2 : SubType;
}

enum InputType
{
    position @0;
    color @1;
    primitiveID @2;
}

struct Input
{
    name @0 : Text;
    type @1 : InputType;
}

enum OutputType
{
    color @0;
    depth @1;
}

struct Output
{
    name @0 : Text;
    type @1 : OutputType;
    subType @2 : SubType;
    blender @3 : Blender;
}

struct MaterialDescriptor
{
    name @0 : Text;
    
    parameters @1 : List(Parameter);
    samplers @2 : List(Sampler);

    inputs @3 : List(Input);
    outputs @4 : List(Output);
}

struct Material
{
    descriptor @0 : MaterialDescriptor;
    psBody @1 : Text;
}