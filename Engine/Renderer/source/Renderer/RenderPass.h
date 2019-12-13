#pragma once
#include <Core.h>
#include <vector>
#include "Descriptors/GraphicsPipelineDesc.h"
#include "Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    class RenderLayer;

    // These will live for a long time
    class RenderPass
    {
    public:
        RenderPass(){}

        void SetPipeline(GraphicsPipelineID pipeline);
        void SetPipeline(ComputePipelineID pipeline);

        void AddRenderLayer(RenderLayer* renderLayer);
    private:

    private:
        GraphicsPipelineID _graphicsPipeline = GraphicsPipelineID::Invalid();
        ComputePipelineID _computePipeline = ComputePipelineID::Invalid();
        std::vector<RenderLayer*> _renderLayers;
    };
}