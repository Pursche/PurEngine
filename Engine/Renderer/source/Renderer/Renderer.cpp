#include "Renderer.h"

namespace Renderer
{
    RenderGraph Renderer::CreateRenderGraph(RenderGraphDesc& desc)
    {
        RenderGraph renderGraph;
        renderGraph.Init(desc);

        return renderGraph;
    }

    ImageID Renderer::CreateImage(ImageDesc& /*desc*/)
    {
        return ImageID::Invalid();
    }

    ImageID Renderer::CreatePermanentImage(ImageDesc& /*desc*/)
    {
        return ImageID::Invalid();
    }

    DepthImageID Renderer::CreateDepthImage(DepthImageDesc& /*desc*/)
    {
        return DepthImageID::Invalid();
    }

    DepthImageID Renderer::CreatePermanentDepthImage(DepthImageDesc& /*desc*/)
    {
        return DepthImageID::Invalid();
    }

    GraphicsPipelineID Renderer::CreatePipeline(GraphicsPipelineDesc& /*desc*/)
    {
        return GraphicsPipelineID::Invalid();
    }

    ComputePipelineID Renderer::CreatePipeline(ComputePipelineDesc& /*desc*/)
    {
        return ComputePipelineID::Invalid();
    }

    RenderLayer Renderer::GetRenderLayer(u32 /*layerHash*/)
    {
        return RenderLayer(); // TODO: Store these in Renderer, assign them names (compiletime hashed) and reuse them
    }

    ModelID Renderer::LoadModel(ModelDesc& /*desc*/)
    {
        return ModelID::Invalid();
    }

    VertexShaderID Renderer::LoadShader(VertexShaderDesc& /*desc*/)
    {
        return VertexShaderID::Invalid();
    }

    PixelShaderID Renderer::LoadShader(PixelShaderDesc& /*desc*/)
    {
        return PixelShaderID::Invalid();
    }

    ComputeShaderID Renderer::LoadShader(ComputeShaderDesc& /*desc*/)
    {
        return ComputeShaderID::Invalid();
    }

}