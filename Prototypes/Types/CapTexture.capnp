 @0xa25e252a362310e8;

enum CapTextureType
{
    simple @0;
}

enum CapChannel
{
    none @0;
    red @1;
    green @2;
    blue @3;
    alpha @4;
}

struct CapTextureHeader
{
    type @0 : CapTextureType;
    width @1 : UInt32;
    height @2 : UInt32;

    rChannel @3 : CapChannel;
    gChannel @4 : CapChannel;
    bChannel @5 : CapChannel;
    aChannel @6 : CapChannel;
}

struct CapTexture
{
    header @0 : CapTextureHeader;
    data @1 : Data;
}