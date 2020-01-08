#include "Renderer.h"

namespace Renderer
{
    RenderGraph Renderer::CreateRenderGraph(RenderGraphDesc& desc)
    {
        RenderGraph renderGraph;
        renderGraph.Init(desc);

        return renderGraph;
    }

    RenderLayer& Renderer::GetRenderLayer(u32 layerHash)
    {
        return _renderLayers[layerHash];
    }
}