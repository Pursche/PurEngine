 @0xa25e252a362310e9;

struct CapVector3
{
    x @0 :Float32;
    y @1 :Float32;
    z @2 :Float32;
}

struct CapModel 
{
    vertexPositions @0 : List(CapVector3);
    indices @1 : List(UInt32);
}