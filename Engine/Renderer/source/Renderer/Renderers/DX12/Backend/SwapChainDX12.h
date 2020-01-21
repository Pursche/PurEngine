#pragma once
#include "../../../SwapChain.h"
#include "d3dx12.h"
#include <dxgi1_4.h>

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;

        struct SwapChainDX12 : public SwapChain
        {
            SwapChainDX12(RenderDeviceDX12* device)
                : swapChain(nullptr)
                , frameIndex(0)
                , bufferCount(2)
                , renderDevice(device)
                , descriptorHeap(nullptr)
                , descriptorSize(0)
            {

            }

            ~SwapChainDX12()
            {
                swapChain.Reset();
                descriptorHeap.Reset();
                for (u32 i = 0; i < bufferCount; i++)
                {
                    resources[i].Reset();
                }
                vertexBuffer.Reset();
                pso.Reset();
                rootSig.Reset();
            }

            Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
            u32 frameIndex;
            u32 bufferCount;

            RenderDeviceDX12* renderDevice;
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
            u32 descriptorSize;

            Microsoft::WRL::ComPtr<ID3D12Resource> resources[3];
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[3];

            Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


            Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
            Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
        };
    }
}