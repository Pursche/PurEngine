#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>

namespace Renderer
{
    struct PrimitivePlaneDesc
    {
        Vector2 size = Vector2(1, 1);
        Vector2 texCoordStart = Vector2(0, 0);
        Vector2 texCoordEnd = Vector2(1, 1);
    };

    // This doesn't need a type since creating it will return a ModelID
}