#include "RenderDeviceDX12.h"
#include "../../../../Window/Window.h"
#include "SwapChainDX12.h"
#include "ConstantBufferDX12.h"

#include "ImageHandlerDX12.h"

#include <cassert>
#include <atlbase.h>

namespace Renderer
{
    namespace Backend
    {
        IDXGIFactory4* RenderDeviceDX12::_dxgiFactory = nullptr;

        void RenderDeviceDX12::Init()
        {
            HRESULT result;

            _frameIndex = 0;

            if (!_dxgiFactory)
            {
                InitOnce();
            }

            IDXGIAdapter1* adapter; // Adapters are the physical graphics card
            int adapterIndex = 0;
            bool adapterFound = false;

            // -- Find first hardware gpu that supports d3d12 --
            while (_dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
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
            assert(adapterFound); // If we could not find a DX12 worthy adapter that's pretty bad.

            // -- Create the Device --
            result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
            assert(SUCCEEDED(result)); // Failed to create device

            // -- Create the Command Queue --
            D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // we will be using all the default values

            result = _device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&_commandQueue)); // create the command queue
            assert(SUCCEEDED(result)); // Failed to create command queue

            // -- Create the Command Allocators -- //
            for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
            {
                result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocators[i]));
                assert(SUCCEEDED(result)); // Failed to create Command Allocator

                result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_resourceCommandAllocators[i]));
                assert(SUCCEEDED(result)); // Failed to create Resource Command Allocator
            }

            // create the command list with the first allocator
            result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocators[0], NULL, IID_PPV_ARGS(&_commandList));
            assert(SUCCEEDED(result)); // Failed to create Command List

            result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _resourceCommandAllocators[0], NULL, IID_PPV_ARGS(&_resourceCommandList));
            assert(SUCCEEDED(result)); // Failed to create Resource Command List

            // command lists are created in the recording state, we will open them before use so we close them now
            _commandList->Close();
            _resourceCommandList->Close();

            // -- Create a Fence & Fence Event -- //

            // create the fences
            for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
            {
                result = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fences[i]));
                assert(SUCCEEDED(result)); // Failed to create fence

                _fenceValues[i] = 0; // set the initial fence value to 0
            }

            // Create constant buffer descriptor heap
            for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = 1;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                result = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_mainDescriptorHeap[i]));
                assert(SUCCEEDED(result)); // Failed to create main descriptor heap
            }
        }

        void RenderDeviceDX12::InitOnce()
        {
            UINT dxgiFactoryFlags = 0;
            
#if defined(_DEBUG)
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
            HRESULT result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_dxgiFactory));
            assert(SUCCEEDED(result)); // Failed to create DXGI factory
        }

        void RenderDeviceDX12::InitWindow(Window* window)
        {
            HRESULT result;
            assert(_dxgiFactory != nullptr); // We need to have initialized the device before initializing a window!

            Vector2i size = window->GetWindowSize();
            const u32 bufferCount = window->GetFrameBufferCount();

            // -- Create our Swap Chain abstraction and give it to the Window
            SwapChainDX12* swapChain = new SwapChainDX12(this);
            window->SetSwapChain(swapChain);
            
            // -- Create the Swap Chain --
            DXGI_MODE_DESC backBufferDesc = {};
            backBufferDesc.Width = size.x;
            backBufferDesc.Height = size.y;
            backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            DXGI_SAMPLE_DESC sampleDesc;
            sampleDesc.Count = 1; // Multisample count, no multisampling
            sampleDesc.Quality = 0;

            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            swapChainDesc.BufferCount = bufferCount;
            swapChainDesc.BufferDesc = backBufferDesc;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.OutputWindow = window->GetHandle();
            swapChainDesc.SampleDesc = sampleDesc;
            swapChainDesc.Windowed = !window->IsFullScreen();

            IDXGISwapChain* tempSwapChain;
            _dxgiFactory->CreateSwapChain(_commandQueue, &swapChainDesc, &tempSwapChain);

            swapChain->swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

            window->SetSwapChain(swapChain);
            swapChain->frameIndex = swapChain->swapChain->GetCurrentBackBufferIndex();

            // -- Create the back buffers --
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = bufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            result = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapChain->descriptorHeap));
            assert(SUCCEEDED(result)); // Failed to create RTV Descriptor Heap

            swapChain->descriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapChain->descriptorHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each buffer
            swapChain->renderTargets = new ID3D12Resource*[bufferCount];

            for (u32 i = 0; i < bufferCount; i++)
            {
                // first we get the n'th buffer in the swap chain and store it in the n'th
               // position of our ID3D12Resource array
                result = swapChain->swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChain->renderTargets[i]));
                assert(SUCCEEDED(result)); // Failed to get RTV from swapchain

                // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
                _device->CreateRenderTargetView(swapChain->renderTargets[i], nullptr, rtvHandle);

                // we increment the rtv handle by the rtv descriptor size we got above
                rtvHandle.Offset(1, swapChain->descriptorSize);
            }
        }

        ID3D12GraphicsCommandList* RenderDeviceDX12::BeginResourceCommandList()
        {
            HRESULT result = _resourceCommandList->Reset(_commandAllocators[_frameIndex], NULL);
            assert(SUCCEEDED(result)); // Failed to reset command list

            return _resourceCommandList;
        }

        void RenderDeviceDX12::EndCommandList(ID3D12GraphicsCommandList* commandList)
        {
            // Execute command list
            commandList->Close();
            ID3D12CommandList* ppCommandLists[] = { commandList };
            _commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        }

        Backend::ConstantBufferBackend* RenderDeviceDX12::CreateConstantBufferBackend(size_t size)
        {
            Backend::ConstantBufferBackendDX12* cbBackend = new Backend::ConstantBufferBackendDX12();

            for (int i = 0; i < Backend::ConstantBufferBackendDX12::FRAME_BUFFER_COUNT; ++i)
            {
                HRESULT result = _device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
                    D3D12_HEAP_FLAG_NONE, // no flags
                    &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
                    D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
                    nullptr, // we do not have use an optimized clear value for constant buffers
                    IID_PPV_ARGS(&cbBackend->uploadHeap[i]));
                assert(SUCCEEDED(result)); // Could not create commited resource for the Constant Buffer

                result = cbBackend->uploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");
                assert(SUCCEEDED(result)); // Could not set name for upload heap

                CD3DX12_RANGE readRange(0, 0);

                // Map the resource heap to get a gpu virtual address to the beginning of the heap
                result = cbBackend->uploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(reinterpret_cast<void**>(&cbBackend->gpuAdress[i])));
                assert(SUCCEEDED(result)); // Could not map to gpu address

                ZeroMemory(cbBackend->gpuAdress[i], size);
            }

            return cbBackend;
        }

        ImageID RenderDeviceDX12::CreateImage(const ImageDesc& desc)
        {
            return _imageHandler->CreateImage(this, desc);
        }

        DepthImageID RenderDeviceDX12::CreateDepthImage(const DepthImageDesc& desc)
        {
            return _imageHandler->CreateDepthImage(this, desc);
        }
    }
}