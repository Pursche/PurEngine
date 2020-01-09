#pragma once
#include <Core.h>
#include <vector>
#include "d3dx12.h"

#include "../../../Descriptors/GraphicsPipelineDesc.h"
#include "../../../Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;
        class ShaderHandlerDX12;

        class PipelineHandlerDX12
        {
            using gIDType = type_safe::underlying_type<GraphicsPipelineID>;
            using cIDType = type_safe::underlying_type<ComputePipelineID>;
        public:
            PipelineHandlerDX12();
            ~PipelineHandlerDX12();

            GraphicsPipelineID CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, const GraphicsPipelineDesc& desc);
            ComputePipelineID CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, const ComputePipelineDesc& desc);

            ID3D12PipelineState* GetPSO(GraphicsPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].pso; }
            ID3D12PipelineState* GetPSO(ComputePipelineID id) { return _computePipelines[static_cast<gIDType>(id)].pso; }

            ID3D12RootSignature* GetRootSignature(GraphicsPipelineID id) { return _graphicsPipelines[static_cast<gIDType>(id)].rootSig; }
            ID3D12RootSignature* GetRootSignature(ComputePipelineID id) { return _computePipelines[static_cast<gIDType>(id)].rootSig; }

        private:
            struct GraphicsPipeline
            {
                GraphicsPipelineDesc desc;
                u64 descHash;

                ID3D12PipelineState* pso;
                ID3D12RootSignature* rootSig;
            };

            struct ComputePipeline
            {
                ComputePipelineDesc desc;
                u64 descHash;

                ID3D12PipelineState* pso;
                ID3D12RootSignature* rootSig;
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

            D3D12_ROOT_SIGNATURE_FLAGS CalculateRootSigFlags(std::vector<D3D12_ROOT_PARAMETER>& parameters);
            
        private:
            std::vector<GraphicsPipeline> _graphicsPipelines;
            std::vector<ComputePipeline> _computePipelines;
        };
    }
}