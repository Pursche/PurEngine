#pragma once
#include <Core.h>
#include "Descriptors/RenderGraphDesc.h"
#include "RenderPass.h"

namespace Renderer
{
    // Acyclic Graph for rendering
    class RenderGraph
    {
    public:
        void AddPass(RenderPass& pass);

        void Compile();
        void Draw();

    private:
        RenderGraph(){} // This gets friend-created by Renderer
        bool Init(RenderGraphDesc& desc);

    private:
        std::vector<RenderPass> _passes;

        friend class Renderer;
    };
}