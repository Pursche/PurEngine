#include "RendererDX12.h"
#include "Backend/d3dx12.h"
#include "Backend/RenderDeviceDX12.h"
#include "Backend/ConstantBufferDX12.h"
#include "Backend/ImageHandlerDX12.h"
#include "Backend/ShaderHandlerDX12.h"
#include "Backend/ModelHandlerDX12.h"
#include "Backend/PipelineHandlerDX12.h"
#include "Backend/CommandListHandlerDX12.h"
#include <WinPixEventRuntime/pix3.h>
#include <Utils/StringUtils.h>

namespace Renderer
{
    RendererDX12::RendererDX12()
        : _device(new Backend::RenderDeviceDX12())
    {
        _device->Init();
        _imageHandler = new Backend::ImageHandlerDX12();
        _shaderHandler = new Backend::ShaderHandlerDX12();
        _modelHandler = new Backend::ModelHandlerDX12();
        _pipelineHandler = new Backend::PipelineHandlerDX12();
        _commandListHandler = new Backend::CommandListHandlerDX12();
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

    GraphicsPipelineID RendererDX12::CreatePipeline(GraphicsPipelineDesc& desc)
    {
        return _pipelineHandler->CreatePipeline(_device, _shaderHandler, _imageHandler, desc);
    }

    ComputePipelineID RendererDX12::CreatePipeline(ComputePipelineDesc& desc)
    {
        return _pipelineHandler->CreatePipeline(_device, _shaderHandler, _imageHandler, desc);
    }

    ModelID RendererDX12::LoadModel(ModelDesc& desc)
    {
        return _modelHandler->LoadModel(_device, _commandListHandler, desc);
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

    CommandListID RendererDX12::BeginCommandList()
    {
        return _commandListHandler->BeginCommandList(_device);
    }

    void RendererDX12::EndCommandList(CommandListID commandList)
    {
        _commandListHandler->EndCommandList(_device, commandList);
    }

    void RendererDX12::Clear(CommandListID commandListID, ImageID imageID, Vector4 color)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = _imageHandler->GetRTV(imageID);

        const float clearColor[] = { color.x, color.y, color.z, color.w };
        commandList->ClearRenderTargetView(rtv, clearColor, 0, NULL);
    }

    D3D12_CLEAR_FLAGS ToClearFlags(DepthClearFlags clearFlags)
    {
        D3D12_CLEAR_FLAGS flags = {};

        switch (clearFlags)
        {
        case DepthClearFlags::DEPTH_CLEAR_DEPTH:
            flags = D3D12_CLEAR_FLAG_DEPTH;
            break;
        case DepthClearFlags::DEPTH_CLEAR_STENCIL:
            flags = D3D12_CLEAR_FLAG_STENCIL;
            break;
        case DepthClearFlags::DEPTH_CLEAR_BOTH:
            flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
            break;
        default:
            assert(false); // Invalid clearFlags value, did we add to the enum?
        }

        return flags; // The enums are identical so just cast it
    }

    void RendererDX12::Clear(CommandListID commandListID, DepthImageID imageID, DepthClearFlags clearFlags, f32 depth, u8 stencil)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = _imageHandler->GetDSV(imageID);
        D3D12_CLEAR_FLAGS flags = ToClearFlags(clearFlags);

        commandList->ClearDepthStencilView(dsv, flags, depth, stencil, 0, NULL);
    }

    void RendererDX12::Draw(CommandListID commandListID, ModelID modelID)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);
        
        D3D12_VERTEX_BUFFER_VIEW* vertexBufferView = _modelHandler->GetVertexBufferView(modelID);
        D3D12_INDEX_BUFFER_VIEW* indexBufferView = _modelHandler->GetIndexBufferView(modelID);
        u32 numIndices = _modelHandler->GetNumIndices(modelID);

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, vertexBufferView);
        commandList->IASetIndexBuffer(indexBufferView);
        commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
    }

    void RendererDX12::PopMarker(CommandListID commandListID)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);
        PIXEndEvent(commandList);
    }

    void RendererDX12::PushMarker(CommandListID commandListID, Vector3 color, std::string name)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        color.x = Math::Clamp(color.x, 0.0f, 1.0f);
        color.y = Math::Clamp(color.y, 0.0f, 1.0f);
        color.z = Math::Clamp(color.z, 0.0f, 1.0f);

        u8 red = static_cast<u8>(color.x * 255);
        u8 green = static_cast<u8>(color.x * 255);
        u8 blue = static_cast<u8>(color.x * 255);

        PIXBeginEvent(commandList, PIX_COLOR(red, green, blue), name.c_str());
    }

    void RendererDX12::SetPipeline(CommandListID /*commandListID*/, GraphicsPipelineID /*pipeline*/)
    {

    }

    void RendererDX12::SetPipeline(CommandListID /*commandListID*/, ComputePipelineID /*pipeline*/)
    {

    }

    Backend::ConstantBufferBackend* RendererDX12::CreateConstantBufferBackend(size_t size)
    {
        return _device->CreateConstantBufferBackend(size);
    }
}