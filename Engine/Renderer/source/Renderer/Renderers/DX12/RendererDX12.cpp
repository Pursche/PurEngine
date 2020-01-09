#include "RendererDX12.h"
#include "Backend/RenderDeviceDX12.h"
#include "Backend/ConstantBufferDX12.h"
#include "Backend/ImageHandlerDX12.h"
#include "Backend/ShaderHandlerDX12.h"
#include "Backend/ModelHandlerDX12.h"

namespace Renderer
{
    RendererDX12::RendererDX12()
        : _device(new Backend::RenderDeviceDX12())
    {
        _device->Init();
        _imageHandler = new Backend::ImageHandlerDX12();
        _shaderHandler = new Backend::ShaderHandlerDX12();
        _modelHandler = new Backend::ModelHandlerDX12();
    }

    void RendererDX12::InitWindow(Window* window)
    {
        _device->InitWindow(window);
    }

    ImageID RendererDX12::CreateImage(ImageDesc& desc)
    {
        return _imageHandler->CreateImage(_device, desc);
    }

    DepthImageID RendererDX12::CreateDepthImage(DepthImageDesc& desc)
    {
        return _imageHandler->CreateDepthImage(_device, desc);
    }

    GraphicsPipelineID RendererDX12::CreatePipeline(GraphicsPipelineDesc& /*desc*/)
    {
        return GraphicsPipelineID::Invalid();
    }

    ComputePipelineID RendererDX12::CreatePipeline(ComputePipelineDesc& /*desc*/)
    {
        return ComputePipelineID::Invalid();
    }

    ModelID RendererDX12::LoadModel(ModelDesc& desc)
    {
        return _modelHandler->LoadModel(_device, desc);
    }

    VertexShaderID RendererDX12::LoadShader(VertexShaderDesc& desc)
    {
        return _shaderHandler->LoadShader(desc);
    }

    PixelShaderID RendererDX12::LoadShader(PixelShaderDesc& desc)
    {
        return _shaderHandler->LoadShader(desc);
    }

    ComputeShaderID RendererDX12::LoadShader(ComputeShaderDesc& desc)
    {
        return _shaderHandler->LoadShader(desc);
    }

    Backend::ConstantBufferBackend* RendererDX12::CreateConstantBufferBackend(size_t size)
    {
        return _device->CreateConstantBufferBackend(size);
    }
}