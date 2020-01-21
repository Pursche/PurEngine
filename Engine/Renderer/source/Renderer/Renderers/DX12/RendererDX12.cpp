#include "RendererDX12.h"
#include "Backend/d3dx12.h"
#include "Backend/RenderDeviceDX12.h"
#include "Backend/ConstantBufferDX12.h"
#include "Backend/ImageHandlerDX12.h"
#include "Backend/ShaderHandlerDX12.h"
#include "Backend/ModelHandlerDX12.h"
#include "Backend/PipelineHandlerDX12.h"
#include "Backend/CommandListHandlerDX12.h"

#include "../../../Window/Window.h"
#include "Backend/SwapChainDX12.h"
#include <WinPixEventRuntime/pix3.h>
#include <wrl/client.h>
#include <Utils/StringUtils.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

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
        _device->InitWindow(_shaderHandler, _commandListHandler, window);
    }

    void RendererDX12::Deinit()
    {
        _device->FlushGPU(); // Make sure it has finished rendering

        delete(_device);
        delete(_imageHandler);
        delete(_shaderHandler);
        delete(_modelHandler);
        delete(_pipelineHandler);
        delete(_commandListHandler);

#ifdef _DEBUG
        Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiDebug.ReleaseAndGetAddressOf()))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
#endif
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

    void RendererDX12::EndCommandList(CommandListID commandListID)
    {
        _commandListHandler->EndCommandList(_device, commandListID);
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

    void RendererDX12::SetConstantBuffer(CommandListID commandListID, u32 slot, void* gpuResource)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        ID3D12Resource* resource = static_cast<ID3D12Resource*>(gpuResource);
        commandList->SetGraphicsRootConstantBufferView(slot, resource->GetGPUVirtualAddress());
    }

    void RendererDX12::SetPipeline(CommandListID commandListID, GraphicsPipelineID pipelineID)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        // Set states
        ID3D12PipelineState* pso = _pipelineHandler->GetPSO(pipelineID);
        ID3D12RootSignature* rootSig = _pipelineHandler->GetRootSignature(pipelineID);

        commandList->SetPipelineState(pso);
        commandList->SetGraphicsRootSignature(rootSig);

        // Set textures
        //commandList->SetGraphicsRootShaderResourceView();

        // Set rendertargets and depthstencil
        const GraphicsPipelineDesc& desc = _pipelineHandler->GetDescriptor(pipelineID);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS] = {};
        u32 boundRenderTargets = 0;
        for (int i = 0; i < MAX_RENDER_TARGETS; i++)
        {
            RenderPassMutableResource resource = desc.renderTargets[i];

            if (resource == RenderPassMutableResource::Invalid())
                break;

            ImageID imageID = desc.MutableResourceToImageID(resource);
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = _imageHandler->GetRTV(imageID);

            rtvs[i] = rtv;
            boundRenderTargets++;
        }

        if (desc.depthStencil != RenderPassMutableResource::Invalid())
        {
            RenderPassMutableResource resource = desc.depthStencil;
            DepthImageID imageID = desc.MutableResourceToDepthImageID(resource);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = _imageHandler->GetDSV(imageID);

            commandList->OMSetRenderTargets(boundRenderTargets, rtvs, false, &dsv);
        }
        else
        {
            commandList->OMSetRenderTargets(boundRenderTargets, rtvs, false, nullptr);
        }
    }

    void RendererDX12::SetPipeline(CommandListID commandListID, ComputePipelineID pipelineID)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);
        ID3D12PipelineState* pso = _pipelineHandler->GetPSO(pipelineID);
        ID3D12RootSignature* rootSig = _pipelineHandler->GetRootSignature(pipelineID);

        commandList->SetPipelineState(pso);
        commandList->SetComputeRootSignature(rootSig);
    }

    void RendererDX12::SetScissorRect(CommandListID commandListID, ScissorRect scissorRect)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        D3D12_RECT rect;
        rect.top = scissorRect.top;
        rect.bottom = scissorRect.bottom;
        rect.left = scissorRect.left;
        rect.right = scissorRect.right;

        commandList->RSSetScissorRects(1, &rect);
    }

    void RendererDX12::SetViewport(CommandListID commandListID, Viewport viewport)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        D3D12_VIEWPORT d3d12Viewport;
        d3d12Viewport.Width = viewport.width;
        d3d12Viewport.Height = viewport.height;
        d3d12Viewport.TopLeftX = viewport.topLeftX;
        d3d12Viewport.TopLeftY = viewport.topLeftY;
        d3d12Viewport.MinDepth = viewport.minDepth;
        d3d12Viewport.MaxDepth = viewport.maxDepth;

        commandList->RSSetViewports(1, &d3d12Viewport);
    }

    void RendererDX12::Present(CommandListID commandListID, Window* window, ImageID image)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        Backend::SwapChainDX12* swapChain = static_cast<Backend::SwapChainDX12*>(window->GetSwapChain());
        u32 frameIndex = swapChain->frameIndex;

        // Transition backbuffer to RenderTarget
        ID3D12Resource* resource = swapChain->resources[frameIndex].Get();
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Transition image to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        ID3D12Resource* srvResource = _imageHandler->GetResource(image);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Set PSO
        commandList->SetPipelineState(swapChain->pso.Get());
        commandList->SetGraphicsRootSignature(swapChain->rootSig.Get());

        // Set SRV descriptor heap
        ID3D12DescriptorHeap* descriptorHeaps[] = { _imageHandler->GetSRVDescriptorHeap(image) };
        commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        for (int i = 0; i < _countof(descriptorHeaps); i++)
        {
            commandList->SetGraphicsRootDescriptorTable(i, descriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart());
        }

        // Set rendertarget
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->rtvs[frameIndex];
        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        // Set viewport and scissor rect
        D3D12_VIEWPORT viewport = {};
        viewport.Width = window->GetWindowSize().x;
        viewport.Height = window->GetWindowSize().y;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1;
        commandList->RSSetViewports(1, &viewport);
        
        D3D12_RECT rect = {};
        rect.right = static_cast<LONG>(viewport.Width);
        rect.bottom = static_cast<LONG>(viewport.Height);
        commandList->RSSetScissorRects(1, &rect);

        // Draw fullscreen quad to backbuffer
        commandList->IASetVertexBuffers(0, 1, &swapChain->vertexBufferView);
        commandList->IASetIndexBuffer(NULL);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        commandList->DrawInstanced(4, 1, 0, 0);

        // Transition backbuffer back to Present
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Transition image back to D3D12_RESOURCE_STATE_RENDER_TARGET
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Execute
        _commandListHandler->EndCommandList(_device, commandListID);

        // Present the current backbuffer
        HRESULT result = swapChain->swapChain->Present(0, 0);
        assert(SUCCEEDED(result));

        // Increment frame index
        swapChain->frameIndex = (swapChain->frameIndex + 1) % swapChain->bufferCount;
    }

    void RendererDX12::Present(CommandListID commandListID, Window* window, DepthImageID image)
    {
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        Backend::SwapChainDX12* swapChain = static_cast<Backend::SwapChainDX12*>(window->GetSwapChain());
        u32 frameIndex = swapChain->frameIndex;

        // Transition backbuffer to RenderTarget
        ID3D12Resource* resource = swapChain->resources[frameIndex].Get();
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Transition image to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        ID3D12Resource* srvResource = _imageHandler->GetResource(image);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Set PSO
        commandList->SetPipelineState(swapChain->pso.Get());
        commandList->SetGraphicsRootSignature(swapChain->rootSig.Get());

        // Set SRV descriptor heap
        ID3D12DescriptorHeap* descriptorHeaps[] = { _imageHandler->GetSRVDescriptorHeap(image) };
        commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        for (int i = 0; i < _countof(descriptorHeaps); i++)
        {
            commandList->SetGraphicsRootDescriptorTable(i, descriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart());
        }

        // Set rendertarget
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->rtvs[frameIndex];
        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        // Draw fullscreen quad to backbuffer
        commandList->IASetVertexBuffers(0, 0, NULL);
        commandList->IASetIndexBuffer(NULL);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(6, 1, 0, 0);

        // Transition backbuffer back to Present
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Transition image back to D3D12_RESOURCE_STATE_DEPTH_WRITE
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        // Execute
        _commandListHandler->EndCommandList(_device, commandListID);

        // Present the current backbuffer
        HRESULT result = swapChain->swapChain->Present(0, 0);
        assert(SUCCEEDED(result));

        // Increment frame index
        swapChain->frameIndex = (swapChain->frameIndex + 1) % swapChain->bufferCount;
    }

    void RendererDX12::Present(Window* window, ImageID image)
    {
        CommandListID commandListID = _commandListHandler->BeginCommandList(_device);
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        Backend::SwapChainDX12* swapChain = static_cast<Backend::SwapChainDX12*>(window->GetSwapChain());
        u32 frameIndex = swapChain->frameIndex;

        // Transition backbuffer to RenderTarget
        ID3D12Resource* resource = swapChain->resources[frameIndex].Get();
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Transition image to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        ID3D12Resource* srvResource = _imageHandler->GetResource(image);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Set PSO
        commandList->SetPipelineState(swapChain->pso.Get());
        commandList->SetGraphicsRootSignature(swapChain->rootSig.Get());

        // Set SRV descriptor heap
        ID3D12DescriptorHeap* descriptorHeaps[] = { _imageHandler->GetSRVDescriptorHeap(image) };
        commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        for (int i = 0; i < _countof(descriptorHeaps); i++)
        {
            commandList->SetGraphicsRootDescriptorTable(i, descriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart());
        }

        // Set rendertarget
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->rtvs[frameIndex];
        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        // Draw fullscreen quad to backbuffer
        commandList->IASetVertexBuffers(0, 0, NULL);
        commandList->IASetIndexBuffer(NULL);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(6, 1, 0, 0);

        // Transition backbuffer back to Present
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Transition image back to D3D12_RESOURCE_STATE_RENDER_TARGET
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Execute
        _commandListHandler->EndCommandList(_device, commandListID);

        // Present the current backbuffer
        HRESULT result = swapChain->swapChain->Present(0, 0);
        assert(SUCCEEDED(result));

        // Increment frame index
        swapChain->frameIndex = (swapChain->frameIndex + 1) % swapChain->bufferCount;
    }

    void RendererDX12::Present(Window* window, DepthImageID image)
    {
        CommandListID commandListID = _commandListHandler->BeginCommandList(_device);
        ID3D12GraphicsCommandList* commandList = _commandListHandler->GetCommandList(commandListID);

        Backend::SwapChainDX12* swapChain = static_cast<Backend::SwapChainDX12*>(window->GetSwapChain());
        u32 frameIndex = swapChain->frameIndex;

        // Transition backbuffer to RenderTarget
        ID3D12Resource* resource = swapChain->resources[frameIndex].Get();
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Transition image to D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        ID3D12Resource* srvResource = _imageHandler->GetResource(image);
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Set PSO
        commandList->SetPipelineState(swapChain->pso.Get());
        commandList->SetGraphicsRootSignature(swapChain->rootSig.Get());

        // Set SRV descriptor heap
        ID3D12DescriptorHeap* descriptorHeaps[] = { _imageHandler->GetSRVDescriptorHeap(image) };
        commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        for (int i = 0; i < _countof(descriptorHeaps); i++)
        {
            commandList->SetGraphicsRootDescriptorTable(i, descriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart());
        }

        // Set rendertarget
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->rtvs[frameIndex];
        commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        // Draw fullscreen quad to backbuffer
        commandList->IASetVertexBuffers(0, 0, NULL);
        commandList->IASetIndexBuffer(NULL);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(6, 1, 0, 0);

        // Transition backbuffer back to Present
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Transition image back to D3D12_RESOURCE_STATE_DEPTH_WRITE
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(srvResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        // Execute
        _commandListHandler->EndCommandList(_device, commandListID);

        // Present the current backbuffer
        HRESULT result = swapChain->swapChain->Present(0, 0);
        assert(SUCCEEDED(result));

        // Increment frame index
        swapChain->frameIndex = (swapChain->frameIndex + 1) % swapChain->bufferCount;
    }

    Backend::ConstantBufferBackend* RendererDX12::CreateConstantBufferBackend(size_t size)
    {
        return _device->CreateConstantBufferBackend(size);
    }
}