#pragma once
#include <Core.h>
#include <Utils/StringUtils.h>
#include "RenderGraph.h"
#include "RenderLayer.h"
#include "RenderPass.h"

// Descriptors
#include "Descriptors/ImageDesc.h"
#include "Descriptors/DepthImageDesc.h"
#include "Descriptors/ModelDesc.h"

namespace Renderer
{
    class Renderer
    {
    public:
        RenderGraph CreateRenderGraph(RenderGraphDesc& desc);

        ImageID CreateImage(ImageDesc& desc);
        ImageID CreatePermanentImage(ImageDesc& desc);

        DepthImageID CreateDepthImage(DepthImageDesc& desc);
        DepthImageID CreatePermanentDepthImage(DepthImageDesc& desc);

        GraphicsPipelineID CreatePipeline(GraphicsPipelineDesc& desc);
        ComputePipelineID CreatePipeline(ComputePipelineDesc& desc);

        RenderLayer GetRenderLayer(u32 layerHash);

        ModelID LoadModel(ModelDesc& desc);

        VertexShaderID LoadShader(VertexShaderDesc& desc);
        PixelShaderID LoadShader(PixelShaderDesc& desc);
        ComputeShaderID LoadShader(ComputeShaderDesc& desc);

    private:

    private:
    };
}