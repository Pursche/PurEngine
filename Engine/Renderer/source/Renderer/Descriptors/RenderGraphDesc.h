#pragma once
#include <Core.h>
#include <vector>

namespace Renderer
{
    class Renderer;

    struct RenderGraphDesc
    {
        std::string name;
        std::vector<std::string> shaderPaths;
        Renderer* renderer;
    };
}