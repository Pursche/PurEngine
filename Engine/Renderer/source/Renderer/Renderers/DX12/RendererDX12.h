#pragma once
#include "../../Renderer.h"

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;
        class ImageHandlerDX12;
        class ShaderHandlerDX12;
        class ModelHandlerDX12;
    }
    
    class RendererDX12 : public Renderer
    {
    public:
        RendererDX12();

        void InitWindow(Window* window) override;

        // Creation
        ImageID CreateImage(ImageDesc& desc) override;
        DepthImageID CreateDepthImage(DepthImageDesc& desc) override;

        GraphicsPipelineID CreatePipeline(GraphicsPipelineDesc& desc) override;
        ComputePipelineID CreatePipeline(ComputePipelineDesc& desc) override;

        // Loading
        ModelID LoadModel(ModelDesc& desc) override;

        VertexShaderID LoadShader(VertexShaderDesc& desc) override;
        PixelShaderID LoadShader(PixelShaderDesc& desc) override;
        ComputeShaderID LoadShader(ComputeShaderDesc& desc) override;

    protected:
        Backend::ConstantBufferBackend* CreateConstantBufferBackend(size_t size) override;

    private:
        Backend::RenderDeviceDX12* _device = nullptr;
        Backend::ImageHandlerDX12* _imageHandler = nullptr;
        Backend::ShaderHandlerDX12* _shaderHandler = nullptr;
        Backend::ModelHandlerDX12* _modelHandler = nullptr;
    };
}
