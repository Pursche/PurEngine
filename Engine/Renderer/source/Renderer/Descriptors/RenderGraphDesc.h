#pragma once
#include <Core.h>
#include <vector>

namespace Memory
{
    class Allocator;
}

namespace Renderer
{
    class Renderer;

    struct RenderGraphDesc
    {
        Memory::Allocator* allocator;
    };
}