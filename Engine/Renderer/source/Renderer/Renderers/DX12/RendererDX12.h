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
        class PipelineHandlerDX12;
        class CommandListHandlerDX12;
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

        // Command List Functions
        CommandListID BeginCommandList() override;
        void EndCommandList(CommandListID commandList) override;
        void Clear(CommandListID commandList, ImageID image, Vector4 color) override;
        void Clear(CommandListID commandList, DepthImageID image, DepthClearFlags clearFlags, f32 depth, u8 stencil) override;
        void Draw(CommandListID commandList, ModelID model) override;
        void PopMarker(CommandListID commandList) override;
        void PushMarker(CommandListID commandList, Vector3 color, std::string name) override;
        void SetPipeline(CommandListID commandList, GraphicsPipelineID pipeline) override;
        void SetPipeline(CommandListID commandList, ComputePipelineID pipeline) override;
        
    protected:
        Backend::ConstantBufferBackend* CreateConstantBufferBackend(size_t size) override;

    private:
        Backend::RenderDeviceDX12* _device = nullptr;
        Backend::ImageHandlerDX12* _imageHandler = nullptr;
        Backend::ShaderHandlerDX12* _shaderHandler = nullptr;
        Backend::ModelHandlerDX12* _modelHandler = nullptr;
        Backend::PipelineHandlerDX12* _pipelineHandler = nullptr;
        Backend::CommandListHandlerDX12* _commandListHandler = nullptr;
    };
}
