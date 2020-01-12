#include "RenderGraph.h"
#include "RenderGraphBuilder.h"

#include "Renderer.h"

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
        CommandList commandList(_renderer);
        for (IRenderPass* pass : _executingPasses)
        {
            pass->Execute(commandList);
        }
        commandList.Execute();

        // TODO: Merge all renderpass commandlists into one and execute
    }

    void RenderGraph::InitializePipelineDesc(GraphicsPipelineDesc& desc)
    {
        desc.ResourceToImageID = [&](RenderPassResource resource) 
        {
            return _renderGraphBuilder->GetImage(resource);
        };
        desc.ResourceToDepthImageID = [&](RenderPassResource resource) 
        {
            return _renderGraphBuilder->GetDepthImage(resource);
        };
        desc.MutableResourceToImageID = [&](RenderPassMutableResource resource) 
        {
            return _renderGraphBuilder->GetImage(resource);
        };
        desc.MutableResourceToDepthImageID = [&](RenderPassMutableResource resource) 
        {
            return _renderGraphBuilder->GetDepthImage(resource);
        };
    }
}