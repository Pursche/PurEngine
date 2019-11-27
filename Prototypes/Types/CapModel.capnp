 @0xa25e252a362310e9;

struct CapVector3
{
    x @0 :Float32;
    y @1 :Float32;
    z @2 :Float32;
}

struct CapVector2
{
    x @0 :Float32;
    y @1 :Float32;
}

struct CapVertex
{
    position @0 :CapVector3;
    normal @1 :CapVector3;
    texCoord @2 :CapVector2;
}

struct CapModel 
{
    vertices @0 :List(CapVertex);
    indices @1 :List(UInt32);
}