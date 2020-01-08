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
                , renderDevice(device)
                , descriptorHeap(nullptr)
                , descriptorSize(0)
                , renderTargets(nullptr)
            {

            }

            IDXGISwapChain3* swapChain;
            u32 frameIndex;

            RenderDeviceDX12* renderDevice;
            ID3D12DescriptorHeap* descriptorHeap;
            u32 descriptorSize;

            ID3D12Resource** renderTargets;
        };
    }
}