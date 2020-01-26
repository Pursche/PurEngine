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
        void Deinit() override;

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
        void EndCommandList(CommandListID commandListID) override;
        void Clear(CommandListID commandListID, ImageID image, Vector4 color) override;
        void Clear(CommandListID commandListID, DepthImageID image, DepthClearFlags clearFlags, f32 depth, u8 stencil) override;
        void Draw(CommandListID commandListID, ModelID model) override;
        void PopMarker(CommandListID commandListID) override;
        void PushMarker(CommandListID commandListID, Vector3 color, std::string name) override;
        void SetConstantBuffer(CommandListID commandListID, u32 slot, void* gpuResource) override;
        void SetPipeline(CommandListID commandListID, GraphicsPipelineID pipeline) override;
        void SetPipeline(CommandListID commandListID, ComputePipelineID pipeline) override;
        void SetScissorRect(CommandListID commandListID, ScissorRect scissorRect) override;
        void SetViewport(CommandListID commandListID, Viewport viewport) override;

        // Non-commandlist based present functions
        void Present(Window* window, ImageID image) override;
        void Present(Window* window, DepthImageID image) override;
        
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
