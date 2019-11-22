#include "RenderDeviceDX12.h"
#include "../../Window/Window.h"
#include "../Model.h"
#include "ShaderHandler.h"
#include "PipelineHandler.h"
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <cassert>
#include <atlbase.h>

RenderDeviceDX12::RenderDeviceDX12()
    : RenderDevice()
    , _frameIndex(0)
{

}

bool RenderDeviceDX12::Init(Window* window, int width, int height)
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    EnableShaderBasedValidation();
    
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

#endif

    HRESULT result;

    IDXGIFactory4* dxgiFactory;
    result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

    if (FAILED(result))
        return false;

    IDXGIAdapter1* adapter; // Adapters are the physical graphics card
    int adapterIndex = 0;
    bool adapterFound = false;

    // -- Find first hardware gpu that supports d3d12 --
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // We don't want a software device
            adapterIndex++;
            continue;
        }

        // We want a device that is compatible with DX12 (feature level 11 or higher)
        result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(result))
        {
            adapterFound = true;
            break;
        }

        adapterIndex++;
    }

    if (!adapterFound)
    {
        return false;
    }

    // -- Create the Device --
    result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));

    if (FAILED(result))
    {
        return false;
    }

    // -- Create the Command Queue --
    D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // we will be using all the default values

    result = _device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&_commandQueue)); // create the command queue
    if (FAILED(result))
    {
        return false;
    }

    // -- Create the Swap Chain --
    DXGI_MODE_DESC backBufferDesc = {};
    backBufferDesc.Width = width;
    backBufferDesc.Height = height;
    backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    _sampleDesc = new DXGI_SAMPLE_DESC();
    _sampleDesc->Count = 1; // Multisample count, no multisampling
    _sampleDesc->Quality = 0;
    
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameBufferCount; 
    swapChainDesc.BufferDesc = backBufferDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = window->GetHandle();
    swapChainDesc.SampleDesc = *_sampleDesc;
    swapChainDesc.Windowed = !window->IsFullScreen();

    IDXGISwapChain* tempSwapChain; // I don't know why this is needed but the sample I looked at did this...
    dxgiFactory->CreateSwapChain(_commandQueue, &swapChainDesc, &tempSwapChain);

    _swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    // -- Create the back buffers --
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    result = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvDescriptorHeap));
    if (FAILED(result))
        return false;

    _rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each buffer
    for (int i = 0; i < frameBufferCount; i++)
    {
        // first we get the n'th buffer in the swap chain and store it in the n'th
       // position of our ID3D12Resource array
        result = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));
        if (FAILED(result))
            return false;

        // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
        _device->CreateRenderTargetView(_renderTargets[i], nullptr, rtvHandle);

        // we increment the rtv handle by the rtv descriptor size we got above
        rtvHandle.Offset(1, _rtvDescriptorSize);
    }

    // -- Create the Command Allocators -- //
    for (int i = 0; i < frameBufferCount; i++)
    {
        result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator[i]));
        if (FAILED(result))
            return false;

        result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_copyCommandAllocator[i]));
        if (FAILED(result))
            return false;
    }

    // create the command list with the first allocator
    result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator[0], NULL, IID_PPV_ARGS(&_commandList));
    if (FAILED(result))
        return false;

    result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _copyCommandAllocator[0], NULL, IID_PPV_ARGS(&_copyCommandList));
    if (FAILED(result))
        return false;
    
    // command lists are created in the recording state. our main loop will set it up for recording again so close it now
    _commandList->Close();
    _copyCommandList->Close();

    // -- Create a Fence & Fence Event -- //

    // create the fences
    for (int i = 0; i < frameBufferCount; i++)
    {
        result = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence[i]));
        if (FAILED(result))
            return false;
        _fenceValue[i] = 0; // set the initial fence value to 0
    }

    // create a handle to a fence event
    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (_fenceEvent == nullptr)
        return false;

    // Fill out the Viewport
    _viewport.TopLeftX = 0;
    _viewport.TopLeftY = 0;
    _viewport.Width = (f32)width;
    _viewport.Height = (f32)height;
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    _scissorRect.left = 0;
    _scissorRect.top = 0;
    _scissorRect.right = width;
    _scissorRect.bottom = height;

    // Load shaders    TODO: Move this out to Renderer or maybe even Demo
    _shaderHandler = new ShaderHandler();
    ShaderHandleVertex vertexShader = _shaderHandler->LoadShaderVertex("Data/shaders/test.vs.hlsl.cso");
    ShaderHandlePixel pixelShader = _shaderHandler->LoadShaderPixel("Data/shaders/test.ps.hlsl.cso");

    // Compile pipeline
    _pipelineHandler = new PipelineHandler(_shaderHandler);

    PipelineDesc pipelineDesc = {};
    pipelineDesc.type = PIPELINE_VERTEX;
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.pixelShader = pixelShader;
    pipelineDesc.sampleDesc = _sampleDesc;

    if (!_pipelineHandler->CreatePipeline(this, pipelineDesc, _pipelineHandle))
        return false;

    // Create constant buffer descriptor heap
    for (int i = 0; i < frameBufferCount; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        result = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_mainDescriptorHeap[i]));
        if (FAILED(result))
            return false;
    }
    
    return true;
}

void RenderDeviceDX12::Render()
{
    HRESULT result;

    UpdatePipeline();

    // create an array of command lists (only one command list here)
    ID3D12CommandList* ppCommandLists[] = { _commandList };

    // execute the array of command lists
    _commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // this command goes in at the end of our command queue. we will know when our command queue 
    // has finished because the fence value will be set to "fenceValue" from the GPU since the command
    // queue is being executed on the GPU
    result = _commandQueue->Signal(_fence[_frameIndex], _fenceValue[_frameIndex]);
    assert(SUCCEEDED(result));

    // present the current backbuffer
    result = _swapChain->Present(0, 0);
    assert(SUCCEEDED(result));
}

void RenderDeviceDX12::Cleanup()
{
    // wait for the gpu to finish all frames
    //for (int i = 0; i < frameBufferCount; ++i)
    //{
    //    _frameIndex = i;
        WaitForFrame();
    //}

    // get swapchain out of full screen before exiting
    BOOL fs = false;
    if (_swapChain->GetFullscreenState(&fs, NULL))
        _swapChain->SetFullscreenState(false, NULL);

    delete _pipelineHandler;
    delete _shaderHandler;

    SAFE_RELEASE(_device);
    SAFE_RELEASE(_swapChain);
    SAFE_RELEASE(_commandQueue);
    SAFE_RELEASE(_rtvDescriptorHeap);
    SAFE_RELEASE(_commandList);

    for (int i = 0; i < frameBufferCount; ++i)
    {
        SAFE_RELEASE(_renderTargets[i]);
        SAFE_RELEASE(_commandAllocator[i]);
        SAFE_RELEASE(_fence[i]);
        SAFE_RELEASE(_mainDescriptorHeap[i]);
    };
}

void RenderDeviceDX12::RegisterModel(Model* model)
{
    if (!model->IsBuffersCreated())
    {
        InitModel(model);
    }

    _modelsToRender.push_back(model);
}

void RenderDeviceDX12::InitModel(Model* model)
{
    HRESULT result;
    result = _commandList->Reset(_commandAllocator[_frameIndex], NULL);
    assert(SUCCEEDED(result));

    // -- VERTEX BUFFER --
    std::vector<Model::Vertex>& vertices = model->GetVertices();
    size_t vertexBufferSize = vertices.size() * sizeof(Model::Vertex);

    // Create vertex buffer
    ID3D12Resource*& vertexBuffer = model->GetVertexBuffer();
    result = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&vertexBuffer));
    assert(SUCCEEDED(result));

    result = vertexBuffer->SetName(L"Vertex Buffer Resource Heap");
    assert(SUCCEEDED(result));

    // Create upload buffer
    ID3D12Resource* vertexBufferUploadHeap;
    result = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&vertexBufferUploadHeap));
    assert(SUCCEEDED(result));
    
    result = vertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
    assert(SUCCEEDED(result));

    // Store vertex buffer in upload buffer
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(model->GetVertices().data()); // pointer to our vertex array
    vertexData.RowPitch = vertexBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vertexBufferSize; // also the size of our triangle vertex data

    // Copy data from upload buffer to vertex buffer
    UpdateSubresources(_commandList, vertexBuffer, vertexBufferUploadHeap, 0, 0, 1, &vertexData);

    // Transition the vertex buffer from copy destination state to vertex buffer state
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    // -- INDEX BUFFER --
    std::vector<u32>& indices = model->GetIndices();
    size_t indexBufferSize = indices.size() * sizeof(u32);

    // Create index buffer
    ID3D12Resource*& indexBuffer = model->GetIndexBuffer();
    result = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&indexBuffer));
    assert(SUCCEEDED(result));

    result = indexBuffer->SetName(L"Index Buffer Resource Heap");
    assert(SUCCEEDED(result));

    // Create upload buffer
    ID3D12Resource* indexBufferUploadHeap;
    result = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&indexBufferUploadHeap));
    assert(SUCCEEDED(result));

    result = indexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
    assert(SUCCEEDED(result));

    // Store index buffer in upload buffer
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(model->GetIndices().data()); // pointer to our index array
    indexData.RowPitch = indexBufferSize; // size of all our triangle index data
    indexData.SlicePitch = indexBufferSize; // also the size of our triangle index data

    // Copy data from upload buffer to index buffer
    UpdateSubresources(_commandList, indexBuffer, indexBufferUploadHeap, 0, 0, 1, &indexData);

    // Transition the index buffer from copy destination state to index buffer state
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    // -- CONSTANT BUFFER --
    for (int i = 0; i < frameBufferCount; ++i)
    {
        ID3D12Resource* constantBufferUploadHeap = model->GetConstantBuffer(i);

        result = _device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&constantBufferUploadHeap));
        assert(SUCCEEDED(result));

        result = constantBufferUploadHeap->SetName(L"Constant Buffer Upload Resource Heap");
        assert(SUCCEEDED(result));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = constantBufferUploadHeap->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(Model::ConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        _device->CreateConstantBufferView(&cbvDesc, _mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

        ZeroMemory(&model->GetConstants(), sizeof(model->GetConstants()));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
        result = constantBufferUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&model->GetConstantBufferGPUAdress(i)));
        assert(SUCCEEDED(result));

        memcpy(model->GetConstantBufferGPUAdress(i), &model->GetConstants(), sizeof(model->GetConstants()));
    }

    // Execute command list
    _commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { _commandList };
    _commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
    _fenceValue[_frameIndex]++;
    result = _commandQueue->Signal(_fence[_frameIndex], _fenceValue[_frameIndex]);
    assert(SUCCEEDED(result));

    // Create a vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW*& vertexBufferView = model->GetVertexBufferView();
    vertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
    vertexBufferView->BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView->StrideInBytes = sizeof(Model::Vertex);
    vertexBufferView->SizeInBytes = (UINT)vertexBufferSize;

    // Create a index buffer view
    D3D12_INDEX_BUFFER_VIEW*& indexBufferView = model->GetIndexBufferView();
    indexBufferView = new D3D12_INDEX_BUFFER_VIEW();
    indexBufferView->BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView->Format = DXGI_FORMAT_R32_UINT;
    indexBufferView->SizeInBytes = (UINT)indexBufferSize;

    model->SetBuffersCreated();
}

void RenderDeviceDX12::EnableShaderBasedValidation()
{
    HRESULT result;

    CComPtr<ID3D12Debug> spDebugController0;
    CComPtr<ID3D12Debug1> spDebugController1;
    result = D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
    assert(SUCCEEDED(result));
    result = spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
    assert(SUCCEEDED(result));
    spDebugController1->SetEnableGPUBasedValidation(true);
}

void RenderDeviceDX12::WaitForFrame()
{
    HRESULT result;

    // swap the current rtv buffer index so we draw on the correct buffer
    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    if (_fence[_frameIndex]->GetCompletedValue() < _fenceValue[_frameIndex])
    {
        // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
        result = _fence[_frameIndex]->SetEventOnCompletion(_fenceValue[_frameIndex], _fenceEvent);
        assert(SUCCEEDED(result));

        // We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
        // has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObject(_fenceEvent, INFINITE);
    }

    // increment fenceValue for next frame
    _fenceValue[_frameIndex]++;
}

void RenderDeviceDX12::UpdatePipeline()
{
    HRESULT result;

    // We have to wait for the gpu to finish with the command allocator before we reset it
    WaitForFrame();

    // we can only reset an allocator once the gpu is done with it
    // resetting an allocator frees the memory that the command list was stored in
    result = _commandAllocator[_frameIndex]->Reset();
    assert(SUCCEEDED(result));

    // reset the command list. by resetting the command list we are putting it into
    // a recording state so we can start recording commands into the command allocator.
    // the command allocator that we reference here may have multiple command lists
    // associated with it, but only one can be recording at any time. Make sure
    // that any other command lists associated to this command allocator are in
    // the closed state (not recording).
    // Here you will pass an initial pipeline state object as the second parameter,
    // but in this tutorial we are only clearing the rtv, and do not actually need
    // anything but an initial default pipeline, which is what we get by setting
    // the second parameter to NULL


    ID3D12PipelineState* pso = _pipelineHandler->GetPSO(_pipelineHandle);
    result = _commandList->Reset(_commandAllocator[_frameIndex], pso);
    assert(SUCCEEDED(result));

    // here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

    // transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), _frameIndex, _rtvDescriptorSize);

    // set the render target for the output merger stage (the output of the pipeline)
    _commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    _commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    ID3D12RootSignature* rootSignature = _pipelineHandler->GetRootSignature(_pipelineHandle);
    _commandList->SetGraphicsRootSignature(rootSignature); // set the root signature
    _commandList->RSSetViewports(1, &_viewport); // set the viewports
    _commandList->RSSetScissorRects(1, &_scissorRect); // set the scissor rects

    // Draw models
    for (Model* model : _modelsToRender)
    {
        // Update model constantbuffer
        memcpy(model->GetConstantBufferGPUAdress(_frameIndex), &model->GetConstants(), sizeof(model->GetConstants()));

        ID3D12DescriptorHeap* descriptorHeaps[] = { _mainDescriptorHeap[_frameIndex] };
        _commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        // set the root descriptor table 0 to the constant buffer descriptor heap
        _commandList->SetGraphicsRootDescriptorTable(0, _mainDescriptorHeap[_frameIndex]->GetGPUDescriptorHandleForHeapStart());

        _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
        _commandList->IASetVertexBuffers(0, 1, model->GetVertexBufferView()); // set the vertex buffer (using the vertex buffer view)
        _commandList->IASetIndexBuffer(model->GetIndexBufferView());
        _commandList->DrawIndexedInstanced(model->GetNumIndices(), 1, 0, 0, 0);
    }

    // transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
    // warning if present is called on the render target when it's not in the present state
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    result = _commandList->Close();
    assert(SUCCEEDED(result));

    _modelsToRender.clear();
}