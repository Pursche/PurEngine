#include "RenderDeviceDX12.h"
#include "../../../../Window/Window.h"
#include "SwapChainDX12.h"
#include "ConstantBufferDX12.h"

#include "ImageHandlerDX12.h"
#include "ShaderHandlerDX12.h"
#include "CommandListHandlerDX12.h"
#include "../../../CommandList.h"

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

            // -- Create a Fence & Fence Event -- //

            // create the fences
            for (int i = 0; i < FRAME_INDEX_COUNT; i++)
            {
                result = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fences[i]));
                assert(SUCCEEDED(result)); // Failed to create fence

                _fenceValues[i] = 0; // set the initial fence value to 0
            }

            // create a handle to a fence event
            _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            assert(_fenceEvent != nullptr); // Failed to create fence event

            // Create constant buffer descriptor heap
            for (int i = 0; i < FRAME_INDEX_COUNT; ++i)
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

        void RenderDeviceDX12::InitWindow(ShaderHandlerDX12* shaderHandler, CommandListHandlerDX12* commandListHandler, Window* window)
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
            swapChain->bufferCount = bufferCount;

            // -- Create the back buffers --
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = bufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            result = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapChain->descriptorHeap));
            assert(SUCCEEDED(result)); // Failed to create RTV Descriptor Heap

            result = swapChain->descriptorHeap->SetName(L"Window Descriptor Heap");
            assert(SUCCEEDED(result)); // Failed to name swapchain RTV 

            swapChain->descriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            // Create a RTV for each buffer
            for (u32 i = 0; i < bufferCount; i++)
            {
                // first we get the n'th buffer in the swap chain and store it in the n'th
               // position of our ID3D12Resource array
                result = swapChain->swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChain->resources[i]));
                assert(SUCCEEDED(result)); // Failed to get RTV from swapchain

                result = swapChain->resources[i]->SetName(L"Window Backbuffer " + i);
                assert(SUCCEEDED(result)); // Failed to name swapchain RTV 

                swapChain->rtvs[i] = swapChain->descriptorHeap->GetCPUDescriptorHandleForHeapStart();
                swapChain->rtvs[i].Offset(i, swapChain->descriptorSize);

                // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
                _device->CreateRenderTargetView(swapChain->resources[i], nullptr, swapChain->rtvs[i]);
            }

            // -- Create present root descriptor --
            // Create SRV descriptor range
            D3D12_DESCRIPTOR_RANGE descriptorTableRange;
            descriptorTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descriptorTableRange.NumDescriptors = 1;
            descriptorTableRange.BaseShaderRegister = 0;
            descriptorTableRange.RegisterSpace = 0;
            descriptorTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            // Create SRV descriptor table
            D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
            descriptorTable.NumDescriptorRanges = 1; // we only have one range
            descriptorTable.pDescriptorRanges = &descriptorTableRange; // the pointer to the beginning of our ranges array

            // Create root parameter
            D3D12_ROOT_PARAMETER srvParameter = {};
            srvParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            srvParameter.DescriptorTable = descriptorTable;
            srvParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            // Create static sampler
            D3D12_STATIC_SAMPLER_DESC sampler = {};
            sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.MipLODBias = 0;
            sampler.MaxAnisotropy = 0;
            sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            sampler.MinLOD = 0.0f;
            sampler.MaxLOD = D3D12_FLOAT32_MAX;
            sampler.ShaderRegister = 0;
            sampler.RegisterSpace = 0;
            sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            // Create root signature
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.Init(1, &srvParameter,
                1, &sampler,
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            );

            ID3DBlob* signature;
            result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
            assert(SUCCEEDED(result)); // Failed to serialize root signature

            result = _device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&swapChain->rootSig));
            assert(SUCCEEDED(result)); // Failed to serialize root signature

            // -- Create input layout --
            D3D12_INPUT_ELEMENT_DESC inputLayout[] = { 
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            // fill out an input layout description structure
            D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
            inputLayoutDesc.NumElements = _countof(inputLayout);
            inputLayoutDesc.pInputElementDescs = inputLayout;

            D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
            depthStencilState.DepthEnable = false;
            depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

            D3D12_RASTERIZER_DESC rasterizerState = {};
            rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            rasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            rasterizerState.FrontCounterClockwise = false;

            // -- create a pipeline state object (PSO) --
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
            psoDesc.pRootSignature = swapChain->rootSig; // the root signature that describes the input data this pso needs
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
            psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
            psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
            psoDesc.RasterizerState = rasterizerState;
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
            psoDesc.NumRenderTargets = 1; // we are only binding one render target
            psoDesc.DepthStencilState = depthStencilState;

            // Bind shaders
            VertexShaderDesc vsDesc;
            vsDesc.path = "Data/shaders/passthrough.vs.hlsl.cso";
            VertexShaderID vsID = shaderHandler->LoadShader(vsDesc);

            PixelShaderDesc psDesc;
            psDesc.path = "Data/shaders/passthrough.ps.hlsl.cso";
            PixelShaderID psID = shaderHandler->LoadShader(psDesc);

            D3D12_SHADER_BYTECODE* vertexShader = shaderHandler->GetBytecode(vsID);
            psoDesc.VS = *vertexShader;
           
            D3D12_SHADER_BYTECODE* pixelShader = shaderHandler->GetBytecode(psID);
            psoDesc.PS = *pixelShader;
            
            // create the pso
            result = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&swapChain->pso));
            assert(SUCCEEDED(result)); // Failed to create PSO

            struct Vertex
            {
                Vertex(Vector3 inPos, Vector2 inTexcoord)
                    : pos(inPos)
                    , texcoord(inTexcoord)
                {}

                Vector3 pos;
                Vector2 texcoord;
            };

            // -- Create quad vertex buffer --
            Vertex vertices[] = 
            {
                // Top left triangle
                Vertex(Vector3(-1.0f, 1.0f, 0.5f), Vector2(0.0f, 0.0f)), // Upper left corner
                Vertex(Vector3(1.0f, 1.0f, 0.5f), Vector2(1.0f, 0.0f)), // Upper right corner
                Vertex(Vector3(-1.0f, -1.0f, 0.5f), Vector2(0.0f, 1.0f)), // Lower left corner
                Vertex(Vector3(1.0f, -1.0f, 0.5f), Vector2(1.0f, 1.0f)) // Lower right corner
            };

            int vBufferSize = sizeof(vertices);

            result = _device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&swapChain->vertexBuffer));
            assert(SUCCEEDED(result)); // Failed to create Vertex Buffer

            result = swapChain->vertexBuffer->SetName(L"SwapChain Vertex Buffer Resource Heap");
            assert(SUCCEEDED(result)); // Failed to name Vertex Buffer

            ID3D12Resource* vBufferUploadHeap;
            result = _device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vBufferUploadHeap));
            assert(SUCCEEDED(result)); // Failed to create Vertex Upload Buffer

            result = vBufferUploadHeap->SetName(L"SwapChain Vertex Buffer Upload Resource Heap");
            assert(SUCCEEDED(result)); // Failed to name Vertex Upload Buffer

            // store vertex buffer in upload heap
            D3D12_SUBRESOURCE_DATA vertexData = {};
            vertexData.pData = reinterpret_cast<BYTE*>(vertices);
            vertexData.RowPitch = vBufferSize;
            vertexData.SlicePitch = vBufferSize;

            CommandListID commandListID = commandListHandler->BeginCommandList(this);
            ID3D12GraphicsCommandList* commandList = commandListHandler->GetCommandList(commandListID);

            UpdateSubresources(commandList, swapChain->vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

            // transition the vertex buffer data from copy destination state to vertex buffer state
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChain->vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

            commandListHandler->EndCommandList(this, commandListID);

            // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
            swapChain->vertexBufferView.BufferLocation = swapChain->vertexBuffer->GetGPUVirtualAddress();
            swapChain->vertexBufferView.StrideInBytes = sizeof(Vertex);
            swapChain->vertexBufferView.SizeInBytes = vBufferSize;
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
                result = cbBackend->uploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(reinterpret_cast<void**>(&cbBackend->gpuAddress[i])));
                assert(SUCCEEDED(result)); // Could not map to gpu address

                ZeroMemory(cbBackend->gpuAddress[i], size);
            }

            return cbBackend;
        }
    }
}