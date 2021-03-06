#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>

namespace Renderer
{
    struct ModelDesc
    {
        std::string path;
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(ModelID, u16);
}