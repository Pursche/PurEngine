#include "RendererDX12.h"
#include "Backend/RenderDeviceDX12.h"
#include "Backend/ConstantBufferDX12.h"

namespace Renderer
{
    RendererDX12::RendererDX12()
        : _device(new Backend::RenderDeviceDX12())
    {
        _device->Init();
    }

    void RendererDX12::InitWindow(Window* window)
    {
        _device->InitWindow(window);
    }

    ImageID RendererDX12::CreateImage(ImageDesc& /*desc*/)
    {
        return ImageID::Invalid();
    }

    DepthImageID RendererDX12::CreateDepthImage(DepthImageDesc& /*desc*/)
    {
        return DepthImageID::Invalid();
    }

    GraphicsPipelineID RendererDX12::CreatePipeline(GraphicsPipelineDesc& /*desc*/)
    {
        return GraphicsPipelineID::Invalid();
    }

    ComputePipelineID RendererDX12::CreatePipeline(ComputePipelineDesc& /*desc*/)
    {
        return ComputePipelineID::Invalid();
    }

    ModelID RendererDX12::LoadModel(ModelDesc& /*desc*/)
    {
        return ModelID::Invalid();
    }

    VertexShaderID RendererDX12::LoadShader(VertexShaderDesc& /*desc*/)
    {
        return VertexShaderID::Invalid();
    }

    PixelShaderID RendererDX12::LoadShader(PixelShaderDesc& /*desc*/)
    {
        return PixelShaderID::Invalid();
    }

    ComputeShaderID RendererDX12::LoadShader(ComputeShaderDesc& /*desc*/)
    {
        return ComputeShaderID::Invalid();
    }

    Backend::ConstantBufferBackend* RendererDX12::CreateConstantBufferBackend(size_t size)
    {
        return _device->CreateConstantBufferBackend(size);
    }
}