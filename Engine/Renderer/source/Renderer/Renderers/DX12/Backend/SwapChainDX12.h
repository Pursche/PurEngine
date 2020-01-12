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

            IDXGISwapChain3* swapChain;
            u32 frameIndex;
            u32 bufferCount;

            RenderDeviceDX12* renderDevice;
            ID3D12DescriptorHeap* descriptorHeap;
            u32 descriptorSize;

            ID3D12Resource* resources[3];
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[3];

            ID3D12PipelineState* pso = nullptr;
            ID3D12RootSignature* rootSig = nullptr;
        };
    }
}