#include "RenderGraph.h"

namespace Renderer
{
    bool RenderGraph::Init(RenderGraphDesc& /*desc*/)
    {

        return true;
    }

    void RenderGraph::AddPass(RenderPass& pass)
    {
        _passes.push_back(pass);
    }

    void RenderGraph::Compile()
    {

    }

    void RenderGraph::Draw()
    {

    }
}