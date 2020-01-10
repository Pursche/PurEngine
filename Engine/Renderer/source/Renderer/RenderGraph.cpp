#include "RenderGraph.h"
#include "RenderGraphBuilder.h"

namespace Renderer
{
    bool RenderGraph::Init(RenderGraphDesc& desc)
    {
        _renderGraphBuilder = new RenderGraphBuilder(desc.renderer);

        return true;
    }

    RenderGraph::~RenderGraph()
    {

    }

    /*void RenderGraph::AddPass(RenderPass& pass)
    {
        _passes.push_back(pass);
    }*/

    void RenderGraph::Setup()
    {
        for (IRenderPass* pass : _passes)
        {
            if (pass->Setup(_renderGraphBuilder))
            {
                _executingPasses.push_back(pass);
            }
        }
    }

    void RenderGraph::Execute()
    {
        // TODO: Parallel_for this
        for (IRenderPass* pass : _executingPasses)
        {
            pass->Execute();
        }

        // TODO: Merge all renderpass commandlists into one and execute
    }
}