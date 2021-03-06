#pragma once
#include <Core.h>
#include <vector>
#include "d3dx12.h"
#include <Containers/RobinHood.h>

#include "../../../Descriptors/MaterialDesc.h"
#include "../../../Descriptors/GraphicsPipelineDesc.h"
#include "../../../Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;
        class ShaderHandlerDX12;
        class ImageHandlerDX12;
        class MaterialHandlerDX12;

        class PipelineHandlerDX12
        {
            using gIDType = type_safe::underlying_type<GraphicsPipelineID>;
            using cIDType = type_safe::underlying_type<ComputePipelineID>;
        public:
            PipelineHandlerDX12();
            ~PipelineHandlerDX12();

            GraphicsPipelineID CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, const GraphicsPipelineDesc& desc);
            MaterialPipelineID CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, MaterialHandlerDX12* materialHandler, const MaterialPipelineDesc& desc);
            ComputePipelineID CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, const ComputePipelineDesc& desc);

            const GraphicsPipelineDesc& GetDescriptor(GraphicsPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].desc; }
            const GraphicsPipelineDesc& GetDescriptor(MaterialPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].desc; }
            const ComputePipelineDesc& GetDescriptor(ComputePipelineID id) { return _computePipelines[static_cast<gIDType>(id)].desc; }

            const MaterialPipelineDesc& GetMaterialDescriptor(MaterialPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].materialDesc; }

            ID3D12PipelineState* GetPSO(GraphicsPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].pso.Get(); }
            ID3D12PipelineState* GetPSO(ComputePipelineID id) { return _computePipelines[static_cast<gIDType>(id)].pso.Get(); }

            ID3D12RootSignature* GetRootSignature(GraphicsPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].rootSig.Get(); }
            ID3D12RootSignature* GetRootSignature(ComputePipelineID id) { return _computePipelines[static_cast<gIDType>(id)].rootSig.Get(); }

        private:
            struct GraphicsPipeline
            {
                GraphicsPipelineDesc desc;
                MaterialPipelineDesc materialDesc;
                u64 cacheDescHash;

                Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;
                Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig = nullptr;
            };
            struct GraphicsPipelineCacheDesc
            {
                GraphicsPipelineDesc::States states;
                u32 numSRVs = 0;
                ImageID renderTargets[MAX_RENDER_TARGETS] = { ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid(), ImageID::Invalid() };
                DepthImageID depthStencil = DepthImageID::Invalid();
            };

            struct ComputePipeline
            {
                ComputePipelineDesc desc;
                u64 cacheDescHash;

                Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;
                Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig = nullptr;
            };

        private:
            bool TryFindExistingGPipeline(u64 descHash, size_t& id);
            bool TryFindExistingCPipeline(u64 descHash, size_t& id);

            D3D12_SHADER_VISIBILITY ToD3D12ShaderVisibility(ShaderVisibility visibility);
            DXGI_FORMAT ToDXGIFormat(InputFormat format);
            D3D12_INPUT_CLASSIFICATION ToD3D12InputClassification(InputClassification classification);

            D3D12_BLEND_DESC ToBlendDesc(const BlendState& blendState);
            D3D12_BLEND ToBlendMode(const BlendMode& blendMode);
            D3D12_BLEND_OP ToBlendOp(const BlendOp& blendOp);
            D3D12_LOGIC_OP ToLogicOp(const LogicOp& logicOp);

            D3D12_RASTERIZER_DESC ToRasterizerDesc(const RasterizerState& rasterizerState);
            D3D12_FILL_MODE ToFillMode(const FillMode& fillMode);
            D3D12_CULL_MODE ToCullMode(const CullMode& cullMode);

            D3D12_DEPTH_STENCIL_DESC ToDepthStencilDesc(const DepthStencilState& depthStencilState);
            D3D12_COMPARISON_FUNC ToComparisonFunc(const ComparisonFunc& comparisonFunc);
            D3D12_DEPTH_STENCILOP_DESC ToDepthStencilOpDesc(const DepthStencilOpDesc& depthStencilOpDesc);
            D3D12_STENCIL_OP ToStencilOp(const StencilOp& op);

            D3D12_FILTER ToFilter(const SamplerFilter& samplerFilter);
            D3D12_TEXTURE_ADDRESS_MODE ToTextureAddressMode(const TextureAddressMode& textureAddressMode);
            D3D12_STATIC_BORDER_COLOR ToStaticBorderColor(const StaticBorderColor& staticBorderColor);

            D3D12_ROOT_SIGNATURE_FLAGS CalculateRootSigFlags(std::vector<D3D12_ROOT_PARAMETER>& parameters);
            DXGI_SAMPLE_DESC CalculateSampleDesc(ImageHandlerDX12* imageHandler, const GraphicsPipelineDesc& desc);

            u64 CalculateCacheDescHash(const GraphicsPipelineDesc& desc);
            
        private:
            std::vector<GraphicsPipeline> _graphicsPipelines;
            std::vector<ComputePipeline> _computePipelines;
        };
    }
}