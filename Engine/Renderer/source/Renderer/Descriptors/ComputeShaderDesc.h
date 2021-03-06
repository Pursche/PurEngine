#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>

namespace Renderer
{
    struct ComputeShaderDesc
    {
        std::string path;
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(ComputeShaderID, u16);
}