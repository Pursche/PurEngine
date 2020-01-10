#include "PipelineHandler.h"
#include "ShaderHandler.h"
#include <cassert>
#include "d3dx12.h"
#include <fstream>
#include "RenderDeviceDX12.h"

PipelineHandler::~PipelineHandler()
{
    for (LoadedPipeline& pipeline : _pipelines)
    {
        SAFE_RELEASE(pipeline.pso);
        SAFE_RELEASE(pipeline.rootSignature);
    }
}

bool PipelineHandler::CreatePipeline(OldRenderDeviceDX12* device, PipelineDesc& desc, PipelineHandle& handle)
{
    handle = PipelineHandle::Invalid();
    HRESULT result;
    size_t nextHandle = _pipelines.size();

    // Make sure we haven't exceeded the limit of the PipelineHandle type, if this hits you need to change type of __PipelineHandle to something bigger
    assert(nextHandle < PipelineHandle::MaxValue());
    using type = type_safe::underlying_type<PipelineHandle>;

    LoadedPipeline loadedPipeline;
    loadedPipeline.handle = PipelineHandle(static_cast<type>(nextHandle));

    // -- create root descriptors --
    D3D12_ROOT_DESCRIPTOR viewCBVDescriptor;
    viewCBVDescriptor.RegisterSpace = 0;
    viewCBVDescriptor.ShaderRegister = 0;

    D3D12_ROOT_DESCRIPTOR modelCBVDescriptor;
    modelCBVDescriptor.RegisterSpace = 0;
    modelCBVDescriptor.ShaderRegister = 1;

    // create a root parameter and fill it out
    D3D12_ROOT_PARAMETER  rootParameters[2]; // two parameters right now
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
    rootParameters[0].Descriptor = viewCBVDescriptor; // this is the root descriptor for this root parameter
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
    rootParameters[1].Descriptor = modelCBVDescriptor; // this is the root descriptor for this root parameter
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameter
        rootParameters, // a pointer to the beginning of our root parameters array
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

    ID3DBlob* signature;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
    if (FAILED(result))
        return false;

    result = device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&loadedPipeline.rootSignature));
    if (FAILED(result))
        return false;

    // -- create input layout --
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

    // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    // -- create a pipeline state object (PSO) --
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = loadedPipeline.rootSignature; // the root signature that describes the input data this pso needs
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
    psoDesc.SampleDesc = *desc.sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target

    // Bind shaders
    if (desc.vertexShader != ShaderHandleVertex::Invalid())
    {
        D3D12_SHADER_BYTECODE* vertexShader = _shaderHandler->GetShaderVertex(desc.vertexShader);
        psoDesc.VS = *vertexShader;
    }
    if (desc.pixelShader != ShaderHandlePixel::Invalid())
    {
        D3D12_SHADER_BYTECODE* pixelShader = _shaderHandler->GetShaderPixel(desc.pixelShader);
        psoDesc.PS = *pixelShader;
    }

    // create the pso
    result = device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&loadedPipeline.pso));
    if (FAILED(result))
        return false;
    

    _pipelines.push_back(loadedPipeline);
    handle = loadedPipeline.handle;
    return true;
}

ID3D12PipelineState* PipelineHandler::GetPSO(PipelineHandle& handle)
{
    using type = type_safe::underlying_type<PipelineHandle>;

    // Lets make sure this handle exists
    assert(_pipelines.size() > static_cast<type>(handle));
    return _pipelines[static_cast<type>(handle)].pso;
}

ID3D12RootSignature* PipelineHandler::GetRootSignature(PipelineHandle& handle)
{
    using type = type_safe::underlying_type<PipelineHandle>;

    // Lets make sure this handle exists
    assert(_pipelines.size() > static_cast<type>(handle));
    return _pipelines[static_cast<type>(handle)].rootSignature;
}

PipelineHandler::ShaderBinary PipelineHandler::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    ShaderBinary buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}