#pragma once
#include <Core.h>
#include <vector>

namespace Renderer
{
    struct RenderGraphDesc
    {
        std::string name;
        std::vector<std::string> shaderPaths;
    };
}