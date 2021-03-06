#pragma once
#include <Core.h>
#include <Utils/StringUtils.h>
#include <Containers/RobinHood.h>
#include "RenderGraph.h"
#include "RenderGraphBuilder.h"
#include "RenderLayer.h"
#include "RenderPass.h"
#include "ConstantBuffer.h"
#include "RenderStates.h"

// Descriptors
#include "Descriptors/CommandListDesc.h"
#include "Descriptors/RendererDesc.h"
#include "Descriptors/VertexShaderDesc.h"
#include "Descriptors/PixelShaderDesc.h"
#include "Descriptors/ComputeShaderDesc.h"
#include "Descriptors/ImageDesc.h"
#include "Descriptors/DepthImageDesc.h"
#include "Descriptors/ModelDesc.h"
#include "Descriptors/PrimitiveModelDesc.h"
#include "Descriptors/MaterialDesc.h"

class Window;

namespace Renderer
{
    class Renderer
    {
    public:
        virtual void InitWindow(Window* window) = 0;
        virtual void Deinit() = 0;

        virtual ~Renderer();

        RenderGraph CreateRenderGraph(RenderGraphDesc& desc);
        RenderLayer& GetRenderLayer(u32 layerHash);

        // Creation
        virtual ImageID CreateImage(ImageDesc& desc) = 0;
        virtual DepthImageID CreateDepthImage(DepthImageDesc& desc) = 0;

        virtual GraphicsPipelineID CreatePipeline(GraphicsPipelineDesc& desc) = 0;
        virtual MaterialPipelineID CreatePipeline(MaterialPipelineDesc& desc) = 0;
        virtual ComputePipelineID CreatePipeline(ComputePipelineDesc& desc) = 0;

        template <typename T>
        ConstantBuffer<T> CreateConstantBuffer()
        {
            ConstantBuffer<T> constantBuffer;
            constantBuffer.backend = CreateConstantBufferBackend(constantBuffer.GetSize());

            return constantBuffer;
        }

        virtual ModelID CreatePrimitiveModel(PrimitivePlaneDesc& desc) = 0;

        // Loading
        virtual TextureID LoadTexture(TextureDesc& desc) = 0;
        virtual ModelID LoadModel(ModelDesc& desc) = 0;
        virtual MaterialID LoadMaterial(MaterialDesc& desc) = 0;

        virtual VertexShaderID LoadShader(VertexShaderDesc& desc) = 0;
        virtual PixelShaderID LoadShader(PixelShaderDesc& desc) = 0;
        virtual ComputeShaderID LoadShader(ComputeShaderDesc& desc) = 0;

        // Command List Functions
        virtual CommandListID BeginCommandList() = 0;
        virtual void EndCommandList(CommandListID commandList) = 0;
        virtual void Clear(CommandListID commandList, ImageID image, Vector4 color) = 0;
        virtual void Clear(CommandListID commandList, DepthImageID image, DepthClearFlags clearFlags, f32 depth, u8 stencil) = 0;
        virtual void Draw(CommandListID commandList, ModelID model) = 0;
        virtual void PopMarker(CommandListID commandList) = 0;
        virtual void PushMarker(CommandListID commandList, Vector3 color, std::string name) = 0;
        virtual void SetConstantBuffer(CommandListID commandList, u32 slot, void* gpuResource) = 0;
        virtual void SetPipeline(CommandListID commandList, GraphicsPipelineID pipeline) = 0;
        virtual void SetPipeline(CommandListID commandList, MaterialPipelineID pipeline) = 0;
        virtual void SetPipeline(CommandListID commandList, ComputePipelineID pipeline) = 0;
        virtual void SetScissorRect(CommandListID commandList, ScissorRect scissorRect) = 0;
        virtual void SetViewport(CommandListID commandList, Viewport viewport) = 0;

        // Non-commandlist based present functions
        virtual void Present(Window* window, ImageID image) = 0;
        virtual void Present(Window* window, DepthImageID image) = 0;

    protected:
        Renderer() {}; // Pure virtual class, disallow creation of it

        virtual Backend::ConstantBufferBackend* CreateConstantBufferBackend(size_t size) = 0;

    protected:
        robin_hood::unordered_map<u32, RenderLayer> _renderLayers;
    };
}