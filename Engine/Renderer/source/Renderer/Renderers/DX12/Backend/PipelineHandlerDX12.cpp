#include "PipelineHandlerDX12.h"
#include "../../../RenderGraph.h"
#include "../../../RenderGraphBuilder.h"
#include "RenderDeviceDX12.h"
#include "ShaderHandlerDX12.h"
#include "ImageHandlerDX12.h"
#include "MaterialHandlerDX12.h"
#include "d3dx12.h"
#include <Utils/StringUtils.h>
#include <Utils/XXHash64.h>
#include <cassert>

namespace Renderer
{
    namespace Backend
    {
        PipelineHandlerDX12::PipelineHandlerDX12()
        {

        }

        PipelineHandlerDX12::~PipelineHandlerDX12()
        {
            for (auto& pipeline : _graphicsPipelines)
            {
                pipeline.rootSig.Reset();
                pipeline.pso.Reset();
            }
            for (auto& pipeline : _computePipelines)
            {
                pipeline.rootSig.Reset();
                pipeline.pso.Reset();
            }
            _graphicsPipelines.clear();
            _computePipelines.clear();
        }

        GraphicsPipelineID PipelineHandlerDX12::CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, const GraphicsPipelineDesc& desc)
        {
            assert(desc.ResourceToImageID != nullptr); // You need to bind this function pointer before creating pipeline, maybe use RenderGraph::InitializePipelineDesc?
            assert(desc.ResourceToDepthImageID != nullptr); // You need to bind this function pointer before creating pipeline, maybe use RenderGraph::InitializePipelineDesc?
            assert(desc.MutableResourceToImageID != nullptr); // You need to bind this function pointer before creating pipeline, maybe use RenderGraph::InitializePipelineDesc?
            assert(desc.MutableResourceToDepthImageID != nullptr); // You need to bind this function pointer before creating pipeline, maybe use RenderGraph::InitializePipelineDesc?

            HRESULT result;
            size_t nextID;

            u64 cacheDescHash = CalculateCacheDescHash(desc);
            
            if (TryFindExistingGPipeline(cacheDescHash, nextID))
            {
                return GraphicsPipelineID(static_cast<gIDType>(nextID));
            }
            
            nextID = _graphicsPipelines.size();

            // Make sure we haven't exceeded the limit of the GraphicsPipelineID type, if this hits you need to change type of GraphicsPipelineID to something bigger
            assert(nextID < GraphicsPipelineID::MaxValue());

            GraphicsPipeline pipeline;
            pipeline.desc = desc;
            pipeline.cacheDescHash = cacheDescHash;

            // -- create root descriptors --
            // Figure out how many we need
            u8 numRootDescriptors = 0;
            u8 numConstantBuffers = 0;
            for (auto& cbState : desc.states.constantBufferStates)
            {
                if (!cbState.enabled)
                    break;

                numRootDescriptors++;
                numConstantBuffers++;
            }

            u8 numSRVs = 0;
            for (auto& texture : desc.textures)
            {
                if (texture == RenderPassResource::Invalid())
                    break;

                numSRVs++;
            }

            // If we have SRVs we need one more root descriptor
            if (numSRVs > 0)
            {
                numRootDescriptors++;
            }
            
            // Create CB root descriptors
            std::vector<D3D12_ROOT_DESCRIPTOR> cbRootDescriptors(numConstantBuffers);
            for(u32 i = 0; i < numConstantBuffers; i++)
            {
                cbRootDescriptors[i].RegisterSpace = 0;
                cbRootDescriptors[i].ShaderRegister = i;
            }

            // Create SRV descriptor range
            D3D12_DESCRIPTOR_RANGE descriptorTableRange;
            descriptorTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descriptorTableRange.NumDescriptors = numSRVs;
            descriptorTableRange.BaseShaderRegister = 0;
            descriptorTableRange.RegisterSpace = 0;
            descriptorTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            // Create SRV descriptor table
            D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
            descriptorTable.NumDescriptorRanges = 1; // we only have one range
            descriptorTable.pDescriptorRanges = &descriptorTableRange; // the pointer to the beginning of our ranges array

            // Create root parameters
            // Create CB root parameters
            std::vector<D3D12_ROOT_PARAMETER> rootParameters(numRootDescriptors);
            for (u32 i = 0; i < numConstantBuffers; i++)
            {
                rootParameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                rootParameters[i].Descriptor = cbRootDescriptors[i];
                rootParameters[i].ShaderVisibility = ToD3D12ShaderVisibility(desc.states.constantBufferStates[i].shaderVisibility);
            }
            // Create SRV root parameters
            if (numSRVs > 0)
            {
                u32 rootParamIndex = numConstantBuffers;
                rootParameters[rootParamIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParameters[rootParamIndex].DescriptorTable = descriptorTable;
                rootParameters[rootParamIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            }

            // Create samplers
            u8 numSamplers = 0;
            for (u32 i = 0; i < MAX_BOUND_TEXTURES; i++)
            {
                if (desc.states.samplers[i].enabled == false)
                    break;

                numSamplers++;
            }

            std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(numSamplers);
            for (int i = 0; i < numSamplers; i++)
            {
                samplers[i].RegisterSpace = 0;
                samplers[i].ShaderRegister = i;
                

                samplers[i].Filter = ToFilter(desc.states.samplers[i].filter);
                samplers[i].AddressU = ToTextureAddressMode(desc.states.samplers[i].addressU);
                samplers[i].AddressV = ToTextureAddressMode(desc.states.samplers[i].addressV);
                samplers[i].AddressW = ToTextureAddressMode(desc.states.samplers[i].addressW);
                samplers[i].MipLODBias = desc.states.samplers[i].mipLODBias;
                samplers[i].MaxAnisotropy = desc.states.samplers[i].maxAnisotropy;
                samplers[i].ComparisonFunc = ToComparisonFunc(desc.states.samplers[i].comparisonFunc);
                samplers[i].BorderColor = ToStaticBorderColor(desc.states.samplers[i].borderColor);
                samplers[i].MinLOD = desc.states.samplers[i].minLOD;
                samplers[i].MaxLOD = desc.states.samplers[i].maxLOD;
                samplers[i].ShaderVisibility = ToD3D12ShaderVisibility(desc.states.samplers[i].shaderVisibility);
            }

            // Create root signature
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init(numRootDescriptors, rootParameters.data(),
                numSamplers, samplers.data(),
                CalculateRootSigFlags(rootParameters)
            );

            Microsoft::WRL::ComPtr<ID3DBlob> signature;
            result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.ReleaseAndGetAddressOf(), nullptr);
            assert(SUCCEEDED(result)); // Failed to serialize root signature

            result = device->_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(pipeline.rootSig.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Failed to serialize root signature

            // -- Create input layout --
            u8 numInputLayouts = 0;
            for (auto& inputLayout : desc.states.inputLayouts)
            {
                if (!inputLayout.enabled)
                    break;

                numInputLayouts++;
            }

            std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayouts(numInputLayouts);
            for (u8 i = 0; i < numInputLayouts; i++)
            {
                inputLayouts[i].SemanticName = desc.states.inputLayouts[i].GetName();
                inputLayouts[i].SemanticIndex = desc.states.inputLayouts[i].index;
                inputLayouts[i].Format = ToDXGIFormat(desc.states.inputLayouts[i].format);
                inputLayouts[i].InputSlot = desc.states.inputLayouts[i].slot;
                inputLayouts[i].AlignedByteOffset = desc.states.inputLayouts[i].alignedByteOffset;
                inputLayouts[i].InputSlotClass = ToD3D12InputClassification(desc.states.inputLayouts[i].inputClassification);
                inputLayouts[i].InstanceDataStepRate = desc.states.inputLayouts[i].instanceDataStepRate;
            }

            // fill out an input layout description structure
            D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
            inputLayoutDesc.NumElements = numInputLayouts;
            inputLayoutDesc.pInputElementDescs = inputLayouts.data();

            // -- create a pipeline state object (PSO) --
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

            // Bind Shaders
            psoDesc.pRootSignature = pipeline.rootSig.Get(); // the root signature that describes the input data this pso needs

            if (desc.states.vertexShader != VertexShaderID::Invalid())
                psoDesc.VS = *static_cast<D3D12_SHADER_BYTECODE*>(shaderHandler->GetBytecode(desc.states.vertexShader));
            if (desc.states.pixelShader != PixelShaderID::Invalid())
                psoDesc.PS = *static_cast<D3D12_SHADER_BYTECODE*>(shaderHandler->GetBytecode(desc.states.pixelShader));
            // TODO(later): psoDesc.DS
            // TODO(later): psoDesc.HS
            // TODO(later): psoDesc.GS

            // Set Render state
            // TODO(maybe): psoDesc.StreamOuptut
            psoDesc.BlendState = ToBlendDesc(desc.states.blendState);
            psoDesc.SampleMask = 0xffffffff; // TODO(maybe): Not hardcode this?
            psoDesc.RasterizerState = ToRasterizerDesc(desc.states.rasterizerState);
            psoDesc.DepthStencilState = ToDepthStencilDesc(desc.states.depthStencilState);
            psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
            // TODO(maybe): psoDesc.IBStripCutValue
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            // Render targets and depth stencil
            for (int i = 0; i < MAX_RENDER_TARGETS; i++)
            {
                if (desc.renderTargets[i] == RenderPassMutableResource::Invalid())
                    break;
                
                ImageID imageId = desc.MutableResourceToImageID(desc.renderTargets[i]);
                psoDesc.RTVFormats[i] = imageHandler->GetDXGIFormat(imageId);
                psoDesc.NumRenderTargets++;
            }
            
            if (desc.depthStencil != RenderPassMutableResource::Invalid())
            {
                DepthImageID depthImageId = desc.MutableResourceToDepthImageID(desc.depthStencil);
                psoDesc.DSVFormat = imageHandler->GetDXGIFormat(depthImageId);
            }
            psoDesc.SampleDesc = CalculateSampleDesc(imageHandler, desc);

            // Other settings I am unsure about
            // TODO(maybe): psoDesc.NodeMask;
            // TODO(maybe): psoDesc.CachedPSO;
            // TODO(maybe): psoDesc.Flags;

            // create the pso
            result = device->_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipeline.pso.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Failed to create PSO

            _graphicsPipelines.push_back(pipeline);

            return GraphicsPipelineID(static_cast<gIDType>(nextID));
        }

        MaterialPipelineID PipelineHandlerDX12::CreatePipeline(RenderDeviceDX12* device, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, MaterialHandlerDX12* materialHandler, const MaterialPipelineDesc& mDesc)
        {
            // A Material pipeline is just a regular Graphics pipeline, but with some of the information supplied by the material
            // So we translate the MaterialPipelineDesc into a GraphicsPipelineDesc here
            GraphicsPipelineDesc gDesc;

            // Shaders from material
            gDesc.states.vertexShader = materialHandler->GetVertexShader(mDesc.material);
            gDesc.states.pixelShader = materialHandler->GetPixelShader(mDesc.material);

            // States
            // From descriptor
            gDesc.states.rasterizerState = mDesc.states.rasterizerState;
            gDesc.states.depthStencilState = mDesc.states.depthStencilState;

            // From material
            gDesc.states.blendState = materialHandler->GetBlendState(mDesc.material);
            
            u8 numConstantBuffers = materialHandler->GetNumConstantBufferStates(mDesc.material);
            for (int i = 0; i < numConstantBuffers; i++)
            {
                gDesc.states.constantBufferStates[i] = materialHandler->GetConstantBufferState(mDesc.material, i);
            }

            u8 numInputLayouts = materialHandler->GetNumInputLayouts(mDesc.material);
            for (int i = 0; i < numInputLayouts; i++)
            {
                gDesc.states.inputLayouts[i] = materialHandler->GetInputLayout(mDesc.material, i);
            }

            u8 numSamplers = materialHandler->GetNumSamplers(mDesc.material);
            for (int i = 0; i < numSamplers; i++)
            {
                gDesc.states.samplers[i] = materialHandler->GetSampler(mDesc.material, i);
            }

            // Functions from descriptor
            gDesc.ResourceToImageID = [](RenderPassResource resource) 
            {
                using resourceType = type_safe::underlying_type<RenderPassResource>;
                return ImageID(static_cast<resourceType>(resource)); // This ONLY works in the case of material textures, everything else should go through the renderpass Setup lambda
            };
            gDesc.ResourceToDepthImageID = mDesc.ResourceToDepthImageID;
            gDesc.MutableResourceToImageID = mDesc.MutableResourceToImageID;
            gDesc.MutableResourceToDepthImageID = mDesc.MutableResourceToDepthImageID;

            u8 numTextures = materialHandler->GetNumTextures(mDesc.material);

            // Textures from descriptor
            for (int i = 0; i < numTextures; i++)
            {
                TextureID texture = materialHandler->GetTexture(mDesc.material, i);
                using _TextureID = type_safe::underlying_type<TextureID>;
                gDesc.textures[i] = RenderPassResource(static_cast<_TextureID>(texture));
            }

            // Rendertargets from descriptor
            for (int i = 0; i < MAX_RENDER_TARGETS; i++)
            {
                gDesc.renderTargets[i] = mDesc.renderTargets[i];
            }
            gDesc.depthStencil = mDesc.depthStencil;

            gIDType pipeline = static_cast<gIDType>(CreatePipeline(device, shaderHandler, imageHandler, gDesc));
            _graphicsPipelines[pipeline].materialDesc = mDesc;
            // And then use the regular CreatePipeline for GraphicsPipelineDesc function
            return MaterialPipelineID(pipeline);
        }

        ComputePipelineID PipelineHandlerDX12::CreatePipeline(RenderDeviceDX12* /*device*/, ShaderHandlerDX12* /*shaderHandler*/, ImageHandlerDX12* /*imageHandler*/, const ComputePipelineDesc& desc)
        {
            //HRESULT result;
            size_t nextID;

            u64 descHash = XXHash64::hash(&desc, sizeof(desc), 0);

            if (TryFindExistingCPipeline(descHash, nextID))
            {
                return ComputePipelineID(static_cast<cIDType>(nextID));
            }

            nextID = _computePipelines.size();

            // Make sure we haven't exceeded the limit of the GraphicsPipelineID type, if this hits you need to change type of GraphicsPipelineID to something bigger
            assert(nextID < ComputePipelineID::MaxValue());

            ComputePipeline pipeline;
            pipeline.desc = desc;
            pipeline.cacheDescHash = descHash;

            // TODO(soon): Create compute pipeline here

            _computePipelines.push_back(pipeline);

            return ComputePipelineID(static_cast<cIDType>(nextID));
        }

        bool PipelineHandlerDX12::TryFindExistingGPipeline(u64 cacheDescHash, size_t& id)
        {
            id = 0;

            for (auto& pipeline : _graphicsPipelines)
            {
                if (cacheDescHash == pipeline.cacheDescHash)
                {
                    return true;
                }
                id++;
            }

            return false;
        }

        bool PipelineHandlerDX12::TryFindExistingCPipeline(u64 cacheDescHash, size_t& id)
        {
            id = 0;

            for (auto& pipeline : _computePipelines)
            {
                if (cacheDescHash == pipeline.cacheDescHash)
                {
                    return true;
                }
                id++;
            }

            return false;
        }

        D3D12_SHADER_VISIBILITY PipelineHandlerDX12::ToD3D12ShaderVisibility(ShaderVisibility visibility)
        {
            switch (visibility)
            {
            case SHADER_VISIBILITY_ALL: return D3D12_SHADER_VISIBILITY_ALL;
            case SHADER_VISIBILITY_VERTEX: return D3D12_SHADER_VISIBILITY_VERTEX;
            case SHADER_VISIBILITY_HULL: return D3D12_SHADER_VISIBILITY_HULL;
            case SHADER_VISIBILITY_DOMAIN: return D3D12_SHADER_VISIBILITY_DOMAIN;
            case SHADER_VISIBILITY_GEOMETRY: return D3D12_SHADER_VISIBILITY_GEOMETRY;
            case SHADER_VISIBILITY_PIXEL: return D3D12_SHADER_VISIBILITY_PIXEL;
            default:
                assert(false); // This should never happen, did we add to the enum?
            }
            
            return D3D12_SHADER_VISIBILITY_ALL;
        }

        DXGI_FORMAT PipelineHandlerDX12::ToDXGIFormat(InputFormat format)
        {
            switch (format)
            {
                case INPUT_FORMAT_UNKNOWN:
                    assert(false); // Unknown format is invalid

                case INPUT_FORMAT_R32G32B32A32_TYPELESS:        return DXGI_FORMAT_R32G32B32A32_TYPELESS;
                case INPUT_FORMAT_R32G32B32A32_FLOAT:           return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case INPUT_FORMAT_R32G32B32A32_UINT:            return DXGI_FORMAT_R32G32B32A32_UINT;
                case INPUT_FORMAT_R32G32B32A32_SINT:            return DXGI_FORMAT_R32G32B32A32_SINT;
                case INPUT_FORMAT_R32G32B32_TYPELESS:           return DXGI_FORMAT_R32G32B32_TYPELESS;
                case INPUT_FORMAT_R32G32B32_FLOAT:              return DXGI_FORMAT_R32G32B32_FLOAT;
                case INPUT_FORMAT_R32G32B32_UINT:               return DXGI_FORMAT_R32G32B32_UINT;
                case INPUT_FORMAT_R32G32B32_SINT:               return DXGI_FORMAT_R32G32B32_SINT;
                case INPUT_FORMAT_R16G16B16A16_TYPELESS:        return DXGI_FORMAT_R16G16B16A16_TYPELESS;
                case INPUT_FORMAT_R16G16B16A16_FLOAT:           return DXGI_FORMAT_R16G16B16A16_FLOAT;
                case INPUT_FORMAT_R16G16B16A16_UNORM:           return DXGI_FORMAT_R16G16B16A16_UNORM;
                case INPUT_FORMAT_R16G16B16A16_UINT:            return DXGI_FORMAT_R16G16B16A16_UINT;
                case INPUT_FORMAT_R16G16B16A16_SNORM:           return DXGI_FORMAT_R16G16B16A16_SNORM;
                case INPUT_FORMAT_R16G16B16A16_SINT:            return DXGI_FORMAT_R16G16B16A16_SINT;
                case INPUT_FORMAT_R32G32_TYPELESS:              return DXGI_FORMAT_R32G32_TYPELESS;
                case INPUT_FORMAT_R32G32_FLOAT:                 return DXGI_FORMAT_R32G32_FLOAT;
                case INPUT_FORMAT_R32G32_UINT:                  return DXGI_FORMAT_R32G32_UINT;
                case INPUT_FORMAT_R32G32_SINT:                  return DXGI_FORMAT_R32G32_SINT;
                case INPUT_FORMAT_R32G8X24_TYPELESS:            return DXGI_FORMAT_R32G8X24_TYPELESS;
                case INPUT_FORMAT_D32_FLOAT_S8X24_UINT:         return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                case INPUT_FORMAT_R32_FLOAT_X8X24_TYPELESS:     return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                case INPUT_FORMAT_X32_TYPELESS_G8X24_UINT:      return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                case INPUT_FORMAT_R10G10B10A2_TYPELESS:         return DXGI_FORMAT_R10G10B10A2_TYPELESS;
                case INPUT_FORMAT_R10G10B10A2_UNORM:            return DXGI_FORMAT_R10G10B10A2_UNORM;
                case INPUT_FORMAT_R10G10B10A2_UINT:             return DXGI_FORMAT_R10G10B10A2_UINT;
                case INPUT_FORMAT_R11G11B10_FLOAT:              return DXGI_FORMAT_R11G11B10_FLOAT;
                case INPUT_FORMAT_R8G8B8A8_TYPELESS:            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
                case INPUT_FORMAT_R8G8B8A8_UNORM:               return DXGI_FORMAT_R8G8B8A8_UNORM;
                case INPUT_FORMAT_R8G8B8A8_UNORM_SRGB:          return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case INPUT_FORMAT_R8G8B8A8_UINT:                return DXGI_FORMAT_R8G8B8A8_UINT;
                case INPUT_FORMAT_R8G8B8A8_SNORM:               return DXGI_FORMAT_R8G8B8A8_SNORM;
                case INPUT_FORMAT_R8G8B8A8_SINT:                return DXGI_FORMAT_R8G8B8A8_SINT;
                case INPUT_FORMAT_R16G16_TYPELESS:              return DXGI_FORMAT_R16G16_TYPELESS;
                case INPUT_FORMAT_R16G16_FLOAT:                 return DXGI_FORMAT_R16G16_FLOAT;
                case INPUT_FORMAT_R16G16_UNORM:                 return DXGI_FORMAT_R16G16_UNORM;
                case INPUT_FORMAT_R16G16_UINT:                  return DXGI_FORMAT_R16G16_UINT;
                case INPUT_FORMAT_R16G16_SNORM:                 return DXGI_FORMAT_R16G16_SNORM;
                case INPUT_FORMAT_R16G16_SINT:                  return DXGI_FORMAT_R16G16_SINT;
                case INPUT_FORMAT_R32_TYPELESS:                 return DXGI_FORMAT_R32_TYPELESS;
                case INPUT_FORMAT_D32_FLOAT:                    return DXGI_FORMAT_D32_FLOAT;
                case INPUT_FORMAT_R32_FLOAT:                    return DXGI_FORMAT_R32_FLOAT;
                case INPUT_FORMAT_R32_UINT:                     return DXGI_FORMAT_R32_UINT;
                case INPUT_FORMAT_R32_SINT:                     return DXGI_FORMAT_R32_SINT;
                case INPUT_FORMAT_R24G8_TYPELESS:               return DXGI_FORMAT_R24G8_TYPELESS;
                case INPUT_FORMAT_D24_UNORM_S8_UINT:            return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case INPUT_FORMAT_R24_UNORM_X8_TYPELESS:        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                case INPUT_FORMAT_X24_TYPELESS_G8_UINT:         return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
                case INPUT_FORMAT_R8G8_TYPELESS:                return DXGI_FORMAT_R8G8_TYPELESS;
                case INPUT_FORMAT_R8G8_UNORM:                   return DXGI_FORMAT_R8G8_UNORM;
                case INPUT_FORMAT_R8G8_UINT:                    return DXGI_FORMAT_R8G8_UINT;
                case INPUT_FORMAT_R8G8_SNORM:                   return DXGI_FORMAT_R8G8_SNORM;
                case INPUT_FORMAT_R8G8_SINT:                    return DXGI_FORMAT_R8G8_SINT;
                case INPUT_FORMAT_R16_TYPELESS:                 return DXGI_FORMAT_R16_TYPELESS;
                case INPUT_FORMAT_R16_FLOAT:                    return DXGI_FORMAT_R16_FLOAT;
                case INPUT_FORMAT_D16_UNORM:                    return DXGI_FORMAT_D16_UNORM;
                case INPUT_FORMAT_R16_UNORM:                    return DXGI_FORMAT_R16_UNORM;
                case INPUT_FORMAT_R16_UINT:                     return DXGI_FORMAT_R16_UINT;
                case INPUT_FORMAT_R16_SNORM:                    return DXGI_FORMAT_R16_SNORM;
                case INPUT_FORMAT_R16_SINT:                     return DXGI_FORMAT_R16_SINT;
                case INPUT_FORMAT_R8_TYPELESS:                  return DXGI_FORMAT_R8_TYPELESS;
                case INPUT_FORMAT_R8_UNORM:                     return DXGI_FORMAT_R8_UNORM;
                case INPUT_FORMAT_R8_UINT:                      return DXGI_FORMAT_R8_UINT;
                case INPUT_FORMAT_R8_SNORM:                     return DXGI_FORMAT_R8_SNORM;
                case INPUT_FORMAT_R8_SINT:                      return DXGI_FORMAT_R8_SINT;
                case INPUT_FORMAT_A8_UNORM:                     return DXGI_FORMAT_A8_UNORM;
                case INPUT_FORMAT_R1_UNORM:                     return DXGI_FORMAT_R1_UNORM;
                case INPUT_FORMAT_R9G9B9E5_SHAREDEXP:           return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                case INPUT_FORMAT_R8G8_B8G8_UNORM:              return DXGI_FORMAT_R8G8_B8G8_UNORM;
                case INPUT_FORMAT_G8R8_G8B8_UNORM:              return DXGI_FORMAT_G8R8_G8B8_UNORM;
                case INPUT_FORMAT_BC1_TYPELESS:                 return DXGI_FORMAT_BC1_TYPELESS;
                case INPUT_FORMAT_BC1_UNORM:                    return DXGI_FORMAT_BC1_UNORM;
                case INPUT_FORMAT_BC1_UNORM_SRGB:               return DXGI_FORMAT_BC1_UNORM_SRGB;
                case INPUT_FORMAT_BC2_TYPELESS:                 return DXGI_FORMAT_BC2_TYPELESS;
                case INPUT_FORMAT_BC2_UNORM:                    return DXGI_FORMAT_BC2_UNORM;
                case INPUT_FORMAT_BC2_UNORM_SRGB:               return DXGI_FORMAT_BC2_UNORM_SRGB;
                case INPUT_FORMAT_BC3_TYPELESS:                 return DXGI_FORMAT_BC3_TYPELESS;
                case INPUT_FORMAT_BC3_UNORM:                    return DXGI_FORMAT_BC3_UNORM;
                case INPUT_FORMAT_BC3_UNORM_SRGB:               return DXGI_FORMAT_BC3_UNORM_SRGB;
                case INPUT_FORMAT_BC4_TYPELESS:                 return DXGI_FORMAT_BC4_TYPELESS;
                case INPUT_FORMAT_BC4_UNORM:                    return DXGI_FORMAT_BC4_UNORM;
                case INPUT_FORMAT_BC4_SNORM:                    return DXGI_FORMAT_BC4_SNORM;
                case INPUT_FORMAT_BC5_TYPELESS:                 return DXGI_FORMAT_BC5_TYPELESS;
                case INPUT_FORMAT_BC5_UNORM:                    return DXGI_FORMAT_BC5_UNORM;
                case INPUT_FORMAT_BC5_SNORM:                    return DXGI_FORMAT_BC5_SNORM;
                case INPUT_FORMAT_B5G6R5_UNORM:                 return DXGI_FORMAT_B5G6R5_UNORM;
                case INPUT_FORMAT_B5G5R5A1_UNORM:               return DXGI_FORMAT_B5G5R5A1_UNORM;
                case INPUT_FORMAT_B8G8R8A8_UNORM:               return DXGI_FORMAT_B8G8R8A8_UNORM;
                case INPUT_FORMAT_B8G8R8X8_UNORM:               return DXGI_FORMAT_B8G8R8X8_UNORM;
                case INPUT_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:   return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
                case INPUT_FORMAT_B8G8R8A8_TYPELESS:            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
                case INPUT_FORMAT_B8G8R8A8_UNORM_SRGB:          return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                case INPUT_FORMAT_B8G8R8X8_TYPELESS:            return DXGI_FORMAT_B8G8R8X8_TYPELESS;
                case INPUT_FORMAT_B8G8R8X8_UNORM_SRGB:          return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
                case INPUT_FORMAT_BC6H_TYPELESS:                return DXGI_FORMAT_BC6H_TYPELESS;
                case INPUT_FORMAT_BC6H_UF16:                    return DXGI_FORMAT_BC6H_UF16;
                case INPUT_FORMAT_BC6H_SF16:                    return DXGI_FORMAT_BC6H_SF16;
                case INPUT_FORMAT_BC7_TYPELESS:                 return DXGI_FORMAT_BC7_TYPELESS;
                case INPUT_FORMAT_BC7_UNORM:                    return DXGI_FORMAT_BC7_UNORM;
                case INPUT_FORMAT_BC7_UNORM_SRGB:               return DXGI_FORMAT_BC7_UNORM_SRGB;
                case INPUT_FORMAT_AYUV:                         return DXGI_FORMAT_AYUV;
                case INPUT_FORMAT_Y410:                         return DXGI_FORMAT_Y410;
                case INPUT_FORMAT_Y416:                         return DXGI_FORMAT_Y416;
                case INPUT_FORMAT_NV12:                         return DXGI_FORMAT_NV12;
                case INPUT_FORMAT_P010:                         return DXGI_FORMAT_P010;
                case INPUT_FORMAT_P016:                         return DXGI_FORMAT_P016;
                case INPUT_FORMAT_420_OPAQUE:                   return DXGI_FORMAT_420_OPAQUE;
                case INPUT_FORMAT_YUY2:                         return DXGI_FORMAT_YUY2;
                case INPUT_FORMAT_Y210:                         return DXGI_FORMAT_Y210;
                case INPUT_FORMAT_Y216:                         return DXGI_FORMAT_Y216;
                case INPUT_FORMAT_NV11:                         return DXGI_FORMAT_NV11;
                case INPUT_FORMAT_AI44:                         return DXGI_FORMAT_AI44;
                case INPUT_FORMAT_IA44:                         return DXGI_FORMAT_IA44;
                case INPUT_FORMAT_P8:                           return DXGI_FORMAT_P8;
                case INPUT_FORMAT_A8P8:                         return DXGI_FORMAT_A8P8;
                case INPUT_FORMAT_B4G4R4A4_UNORM:               return DXGI_FORMAT_B4G4R4A4_UNORM;
                case INPUT_FORMAT_P208:                         return DXGI_FORMAT_P208;
                case INPUT_FORMAT_V208:                         return DXGI_FORMAT_V208;
                case INPUT_FORMAT_V408:                         return DXGI_FORMAT_V408;
                case INPUT_FORMAT_FORCE_UINT:                   return DXGI_FORMAT_FORCE_UINT;
                default:
                    assert(false); // Invalid input format, did we add something to the enum?
            }

            return DXGI_FORMAT_UNKNOWN;
        }

        D3D12_INPUT_CLASSIFICATION PipelineHandlerDX12::ToD3D12InputClassification(InputClassification classification)
        {
            switch (classification)
            {
                case INPUT_CLASSIFICATION_PER_VERTEX: return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                case INPUT_CLASSIFICATION_PER_INSTANCE: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                default:
                    assert(false); // Invalid input classification, did we add something to the enum?
            }
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        }

        D3D12_BLEND_DESC PipelineHandlerDX12::ToBlendDesc(const BlendState& blendState)
        {
            D3D12_BLEND_DESC blendDesc;

            blendDesc.AlphaToCoverageEnable = blendState.alphaToCoverageEnable;
            blendDesc.IndependentBlendEnable = blendState.independentBlendEnable;

            for (int i = 0; i < MAX_RENDER_TARGETS; i++)
            {
                blendDesc.RenderTarget[i].BlendEnable = blendState.renderTargets[i].blendEnable;
                blendDesc.RenderTarget[i].LogicOpEnable = blendState.renderTargets[i].logicOpEnable;
                blendDesc.RenderTarget[i].SrcBlend = ToBlendMode(blendState.renderTargets[i].srcBlend);
                blendDesc.RenderTarget[i].DestBlend = ToBlendMode(blendState.renderTargets[i].destBlend);
                blendDesc.RenderTarget[i].BlendOp = ToBlendOp(blendState.renderTargets[i].blendOp);
                blendDesc.RenderTarget[i].SrcBlendAlpha = ToBlendMode(blendState.renderTargets[i].srcBlendAlpha);
                blendDesc.RenderTarget[i].DestBlendAlpha = ToBlendMode(blendState.renderTargets[i].destBlendAlpha);
                blendDesc.RenderTarget[i].BlendOpAlpha = ToBlendOp(blendState.renderTargets[i].blendOpAlpha);
                blendDesc.RenderTarget[i].LogicOp = ToLogicOp(blendState.renderTargets[i].logicOp);
                blendDesc.RenderTarget[i].RenderTargetWriteMask = blendState.renderTargets[i].renderTargetWriteMask;
            }

            return blendDesc;
        }

        D3D12_BLEND PipelineHandlerDX12::ToBlendMode(const BlendMode& blendMode)
        {
            switch (blendMode)
            {
                case BLEND_MODE_ZERO:               return D3D12_BLEND_ZERO;
                case BLEND_MODE_ONE:                return D3D12_BLEND_ONE;
                case BLEND_MODE_SRC_COLOR:          return D3D12_BLEND_SRC_COLOR;
                case BLEND_MODE_INV_SRC_COLOR:      return D3D12_BLEND_INV_SRC_COLOR;
                case BLEND_MODE_SRC_ALPHA:          return D3D12_BLEND_SRC_ALPHA;
                case BLEND_MODE_INV_SRC_ALPHA:      return D3D12_BLEND_INV_SRC_ALPHA;
                case BLEND_MODE_DEST_ALPHA:         return D3D12_BLEND_DEST_ALPHA;
                case BLEND_MODE_INV_DEST_ALPHA:     return D3D12_BLEND_INV_DEST_ALPHA;
                case BLEND_MODE_DEST_COLOR:         return D3D12_BLEND_DEST_COLOR;
                case BLEND_MODE_INV_DEST_COLOR:     return D3D12_BLEND_INV_DEST_COLOR;
                case BLEND_MODE_SRC_ALPHA_SAT:      return D3D12_BLEND_SRC_ALPHA_SAT;
                case BLEND_MODE_BLEND_FACTOR:       return D3D12_BLEND_BLEND_FACTOR;
                case BLEND_MODE_INV_BLEND_FACTOR:   return D3D12_BLEND_INV_BLEND_FACTOR;
                case BLEND_MODE_SRC1_COLOR:         return D3D12_BLEND_SRC1_COLOR;
                case BLEND_MODE_INV_SRC1_COLOR:     return D3D12_BLEND_INV_SRC1_COLOR;
                case BLEND_MODE_SRC1_ALPHA:         return D3D12_BLEND_SRC1_ALPHA;
                case BLEND_MODE_INV_SRC1_ALPHA:     return D3D12_BLEND_INV_SRC1_ALPHA;
                default:
                    assert(false); // Invalid blend mode, did we add something to the enum?
            }

            return D3D12_BLEND_ZERO;
        }

        D3D12_BLEND_OP PipelineHandlerDX12::ToBlendOp(const BlendOp& blendOp)
        {
            switch (blendOp)
            {
            case BLEND_OP_ADD:              return D3D12_BLEND_OP_ADD;
            case BLEND_OP_SUBTRACT:         return D3D12_BLEND_OP_SUBTRACT;
            case BLEND_OP_REV_SUBTRACT:     return D3D12_BLEND_OP_REV_SUBTRACT;
            case BLEND_OP_MIN:              return D3D12_BLEND_OP_MIN;
            case BLEND_OP_MAX:              return D3D12_BLEND_OP_MAX;
            default:
                assert(false); // Invalid blend op, did we add something to the enum?
            }

            return D3D12_BLEND_OP_ADD;
        }

        D3D12_LOGIC_OP PipelineHandlerDX12::ToLogicOp(const LogicOp& logicOp)
        {
            switch (logicOp)
            {
                case LOGIC_OP_CLEAR:            return D3D12_LOGIC_OP_CLEAR;
                case LOGIC_OP_SET:              return D3D12_LOGIC_OP_SET;
                case LOGIC_OP_COPY:             return D3D12_LOGIC_OP_COPY;
                case LOGIC_OP_COPY_INVERTED:    return D3D12_LOGIC_OP_COPY_INVERTED;
                case LOGIC_OP_NOOP:             return D3D12_LOGIC_OP_NOOP;
                case LOGIC_OP_INVERT:           return D3D12_LOGIC_OP_INVERT;
                case LOGIC_OP_AND:              return D3D12_LOGIC_OP_AND;
                case LOGIC_OP_NAND:             return D3D12_LOGIC_OP_NAND;
                case LOGIC_OP_OR:               return D3D12_LOGIC_OP_OR;
                case LOGIC_OP_NOR:              return D3D12_LOGIC_OP_NOR;
                case LOGIC_OP_XOR:              return D3D12_LOGIC_OP_XOR;
                case LOGIC_OP_EQUIV:            return D3D12_LOGIC_OP_EQUIV;
                case LOGIC_OP_AND_REVERSE:      return D3D12_LOGIC_OP_AND_REVERSE;
                case LOGIC_OP_AND_INVERTED:     return D3D12_LOGIC_OP_AND_INVERTED;
                case LOGIC_OP_OR_REVERSE:       return D3D12_LOGIC_OP_OR_REVERSE;
                case LOGIC_OP_OR_INVERTED:      return D3D12_LOGIC_OP_OR_INVERTED;
                default:
                    assert(false); // Invalid logic op, did we add something to the enum?
            }

            return D3D12_LOGIC_OP_CLEAR;
        }

        D3D12_RASTERIZER_DESC PipelineHandlerDX12::ToRasterizerDesc(const RasterizerState& state)
        {
            D3D12_RASTERIZER_DESC desc;
            desc.FillMode = ToFillMode(state.fillMode);
            desc.CullMode = ToCullMode(state.cullMode);
            desc.FrontCounterClockwise = state.frontFaceMode;
            desc.DepthBias = state.depthBias;
            desc.DepthBiasClamp = state.depthBiasClamp;
            desc.SlopeScaledDepthBias = state.depthBiasSlopeFactor;
            desc.DepthClipEnable = state.depthBiasEnabled;
            desc.MultisampleEnable = state.sampleCount != SAMPLE_COUNT_1;
            desc.AntialiasedLineEnable = false; // TODO: MSAA support
            desc.ForcedSampleCount = state.sampleCount;
            desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // TODO: MSAA support

            return desc;
        }

        D3D12_FILL_MODE PipelineHandlerDX12::ToFillMode(const FillMode& fillMode)
        {
            switch (fillMode)
            {
                case FILL_MODE_SOLID:       return D3D12_FILL_MODE_SOLID;
                case FILL_MODE_WIREFRAME:   return D3D12_FILL_MODE_WIREFRAME;
                default:
                    assert(false); // Invalid fill mode, did we add something to the enum?
            }

            return D3D12_FILL_MODE_SOLID;
        }

        D3D12_CULL_MODE PipelineHandlerDX12::ToCullMode(const CullMode& cullMode)
        {
            switch (cullMode)
            {
                case CULL_MODE_NONE:    return D3D12_CULL_MODE_NONE;
                case CULL_MODE_FRONT:   return D3D12_CULL_MODE_FRONT;
                case CULL_MODE_BACK:    return D3D12_CULL_MODE_BACK;
                default:
                    assert(false); // Invalid cull mode, did we add something to the enum?
            }

            return D3D12_CULL_MODE_NONE;
        }

        D3D12_DEPTH_STENCIL_DESC PipelineHandlerDX12::ToDepthStencilDesc(const DepthStencilState& state)
        {
            D3D12_DEPTH_STENCIL_DESC desc;
            desc.DepthEnable = state.depthEnable;
            desc.DepthWriteMask = state.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = ToComparisonFunc(state.depthFunc);
            desc.StencilEnable = state.stencilEnable;
            desc.StencilReadMask = state.stencilReadMask;
            desc.StencilWriteMask = state.stencilWriteMask;
            desc.FrontFace = ToDepthStencilOpDesc(state.frontFace);
            desc.BackFace = ToDepthStencilOpDesc(state.backFace);

            return desc;
        }

        D3D12_COMPARISON_FUNC PipelineHandlerDX12::ToComparisonFunc(const ComparisonFunc& comparisonFunc)
        {
            switch (comparisonFunc)
            {
                case COMPARISON_FUNC_NEVER:         return D3D12_COMPARISON_FUNC_NEVER;
                case COMPARISON_FUNC_LESS:          return D3D12_COMPARISON_FUNC_LESS;
                case COMPARISON_FUNC_EQUAL:         return D3D12_COMPARISON_FUNC_EQUAL;
                case COMPARISON_FUNC_LESS_EQUAL:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case COMPARISON_FUNC_GREATER:       return D3D12_COMPARISON_FUNC_GREATER;
                case COMPARISON_FUNC_NOT_EQUAL:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case COMPARISON_FUNC_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case COMPARISON_FUNC_ALWAYS:        return D3D12_COMPARISON_FUNC_ALWAYS;
                default:
                    assert(false); // Invalid comparison func, did we add something to the enum?
            }

            return D3D12_COMPARISON_FUNC_NEVER;
        }

        D3D12_DEPTH_STENCILOP_DESC PipelineHandlerDX12::ToDepthStencilOpDesc(const DepthStencilOpDesc& depthStencilOpDesc)
        {
            D3D12_DEPTH_STENCILOP_DESC desc;
            desc.StencilFailOp = ToStencilOp(depthStencilOpDesc.stencilFailOp);
            desc.StencilDepthFailOp = ToStencilOp(depthStencilOpDesc.stencilDepthFailOp);
            desc.StencilPassOp = ToStencilOp(depthStencilOpDesc.stencilPassOp);
            desc.StencilFunc = ToComparisonFunc(depthStencilOpDesc.stencilFunc);

            return desc;
        }

        D3D12_STENCIL_OP PipelineHandlerDX12::ToStencilOp(const StencilOp& op)
        {
            switch (op)
            {
                case STENCIL_OP_KEEP:     return D3D12_STENCIL_OP_KEEP;
                case STENCIL_OP_ZERO:     return D3D12_STENCIL_OP_ZERO;
                case STENCIL_OP_REPLACE:  return D3D12_STENCIL_OP_REPLACE;
                case STENCIL_OP_INCR_SAT: return D3D12_STENCIL_OP_INCR_SAT;
                case STENCIL_OP_DECR_SAT: return D3D12_STENCIL_OP_DECR_SAT;
                case STENCIL_OP_INVERT:   return D3D12_STENCIL_OP_INVERT;
                case STENCIL_OP_INCR:     return D3D12_STENCIL_OP_INCR;
                case STENCIL_OP_DECR:     return D3D12_STENCIL_OP_DECR;
                default:
                    assert(false); // Invalid stencil op, did we add something to the enum?
            }

            return D3D12_STENCIL_OP_KEEP;
        }

        D3D12_FILTER PipelineHandlerDX12::ToFilter(const SamplerFilter& samplerFilter)
        {
            switch (samplerFilter)
            {
                case SAMPLER_FILTER_MIN_MAG_MIP_POINT:                          return D3D12_FILTER_MIN_MAG_MIP_POINT;
                case SAMPLER_FILTER_MIN_MAG_POINT_MIP_LINEAR:                   return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:             return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MIN_POINT_MAG_MIP_LINEAR:                   return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_MIN_LINEAR_MAG_MIP_POINT:                   return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                case SAMPLER_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:            return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MIN_MAG_LINEAR_MIP_POINT:                   return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MIN_MAG_MIP_LINEAR:                         return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_ANISOTROPIC:                                return D3D12_FILTER_ANISOTROPIC;
                case SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_POINT:               return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                case SAMPLER_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:        return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:  return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:        return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:        return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                case SAMPLER_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR: return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:        return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:              return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_COMPARISON_ANISOTROPIC:                     return D3D12_FILTER_COMPARISON_ANISOTROPIC;
                case SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_POINT:                  return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
                case SAMPLER_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:           return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:     return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:           return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:           return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
                case SAMPLER_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:    return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:           return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:                 return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_MINIMUM_ANISOTROPIC:                        return D3D12_FILTER_MINIMUM_ANISOTROPIC;
                case SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_POINT:                  return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
                case SAMPLER_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:           return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:     return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:           return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:           return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
                case SAMPLER_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:    return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                case SAMPLER_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:           return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
                case SAMPLER_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR:                 return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
                case SAMPLER_FILTER_MAXIMUM_ANISOTROPIC:                        return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
                default:
                    assert(false); // Did we change SamplerFilter without updating this function?
            }

            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        }

        D3D12_TEXTURE_ADDRESS_MODE PipelineHandlerDX12::ToTextureAddressMode(const TextureAddressMode& textureAddressMode)
        {
            switch (textureAddressMode)
            {
                case TEXTURE_ADDRESS_MODE_WRAP:         return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case TEXTURE_ADDRESS_MODE_MIRROR:       return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case TEXTURE_ADDRESS_MODE_CLAMP:        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case TEXTURE_ADDRESS_MODE_BORDER:       return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                case TEXTURE_ADDRESS_MODE_MIRROR_ONCE:  return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
                default:
                    assert(false); // Did we change TextureAddressMode without updating this function?
            }

            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }

        D3D12_STATIC_BORDER_COLOR PipelineHandlerDX12::ToStaticBorderColor(const StaticBorderColor& staticBorderColor)
        {
            switch (staticBorderColor)
            {
                case STATIC_BORDER_COLOR_TRANSPARENT_BLACK:     return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                case STATIC_BORDER_COLOR_OPAQUE_BLACK:          return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
                case STATIC_BORDER_COLOR_OPAQUE_WHITE:          return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
                default:
                    assert(false); // Did we change StaticBorderColor without updating this function?
            }

            return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        }

        D3D12_ROOT_SIGNATURE_FLAGS PipelineHandlerDX12::CalculateRootSigFlags(std::vector<D3D12_ROOT_PARAMETER>& parameters)
        {
            D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            bool hasVertexVisibility = false;
            bool hasHullVisibility = false;
            bool hasDomainVisibility = false;
            bool hasGeometryVisibility = false;
            bool hasPixelVisibility = false;

            for (D3D12_ROOT_PARAMETER& param : parameters)
            {
                if (param.ShaderVisibility == D3D12_SHADER_VISIBILITY_ALL)
                {
                    hasVertexVisibility = true;
                    hasHullVisibility = true;
                    hasDomainVisibility = true;
                    hasGeometryVisibility = true;
                    hasPixelVisibility = true;
                    break;
                }

                switch (param.ShaderVisibility)
                {
                    case D3D12_SHADER_VISIBILITY_VERTEX:
                        hasVertexVisibility = true;
                        break;
                    case D3D12_SHADER_VISIBILITY_HULL:
                        hasHullVisibility = true;
                        break;
                    case D3D12_SHADER_VISIBILITY_DOMAIN:
                        hasDomainVisibility = true;
                        break;
                    case D3D12_SHADER_VISIBILITY_GEOMETRY:
                        hasGeometryVisibility = true;
                        break;
                    case D3D12_SHADER_VISIBILITY_PIXEL:
                        hasPixelVisibility = true;
                        break;
                }
            }

            if (!hasVertexVisibility)
                flags = flags | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
            if (!hasHullVisibility)
                flags = flags | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
            if (!hasDomainVisibility)
                flags = flags | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
            if (!hasGeometryVisibility)
                flags = flags | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            if (!hasPixelVisibility)
                flags = flags | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

            return flags;
        }

        DXGI_SAMPLE_DESC PipelineHandlerDX12::CalculateSampleDesc(ImageHandlerDX12* imageHandler, const GraphicsPipelineDesc& desc)
        {
            DXGI_SAMPLE_DESC sampleDesc;

            u32 sampleCount = std::numeric_limits<u32>::max();

            for (auto& renderTarget : desc.renderTargets)
            {
                if (renderTarget == RenderPassMutableResource::Invalid())
                    break;

                ImageID imageID = desc.MutableResourceToImageID(renderTarget);
                const ImageDesc& descriptor = imageHandler->GetDescriptor(imageID);

                u32 descSampleCount = SampleCountToInt(descriptor.sampleCount);

                if (sampleCount != std::numeric_limits<u32>::max() && sampleCount != descSampleCount)
                {
                    assert(false); // It seems like we bound rendertargets (or depthstencil) with different sample counts, that's a big no-no
                }
                
                sampleCount = descSampleCount;
            }

            if (desc.depthStencil != RenderPassMutableResource::Invalid())
            {
                DepthImageID imageID = desc.MutableResourceToDepthImageID(desc.depthStencil);
                const DepthImageDesc& descriptor = imageHandler->GetDescriptor(imageID);

                u32 descSampleCount = SampleCountToInt(descriptor.sampleCount);

                if (sampleCount != std::numeric_limits<u32>::max() && sampleCount != descSampleCount)
                {
                    assert(false); // It seems like we bound rendertargets (or depthstencil) with different sample counts, that's a big no-no
                }

                sampleCount = descSampleCount;
            }

            sampleDesc.Count = sampleCount;
            sampleDesc.Quality = (sampleCount == 1) ? 0 : sampleCount;

            return sampleDesc;
        }

        u64 PipelineHandlerDX12::CalculateCacheDescHash(const GraphicsPipelineDesc& desc)
        {
            GraphicsPipelineCacheDesc cacheDesc;
            cacheDesc.states = desc.states;

            cacheDesc.numSRVs = 0;
            for (auto& texture : desc.textures)
            {
                if (texture == RenderPassResource::Invalid())
                    break;

                cacheDesc.numSRVs++;
            }

            for (int i = 0; i < MAX_RENDER_TARGETS; i++)
            {
                if (desc.renderTargets[i] == RenderPassMutableResource::Invalid())
                    break;

                cacheDesc.renderTargets[i] = desc.MutableResourceToImageID(desc.renderTargets[i]);
            }

            if (desc.depthStencil != RenderPassMutableResource::Invalid())
            {
                cacheDesc.depthStencil = desc.MutableResourceToDepthImageID(desc.depthStencil);
            }

            u64 hash = XXHash64::hash(&cacheDesc, sizeof(cacheDesc), 0);

            return hash;
        }
    }
}