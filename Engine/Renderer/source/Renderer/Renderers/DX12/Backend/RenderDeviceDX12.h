#pragma once
#include <Core.h>
#include "d3dx12.h"
#include <dxgi1_4.h>

#include "../../../Descriptors/ImageDesc.h"
#include "../../../Descriptors/DepthImageDesc.h"

class Window;

namespace Renderer
{
    class CommandList;

    namespace Backend
    {
        struct ConstantBufferBackend;
        class ShaderHandlerDX12;
        class CommandListHandlerDX12;
        struct SwapChainDX12;

        class RenderDeviceDX12
        {
        public:
            ~RenderDeviceDX12();

            void Init();
            void InitWindow(ShaderHandlerDX12* shaderHandler, CommandListHandlerDX12* commandListHandler, Window* window);

            ConstantBufferBackend* CreateConstantBufferBackend(size_t size);

            u32 GetFrameIndex() { return _frameIndex; }
            void EndFrame() { _frameIndex = (_frameIndex + 1) % FRAME_INDEX_COUNT; }

            void FlushGPU();

        private:
            void InitOnce();
            static Microsoft::WRL::ComPtr<IDXGIFactory4> _dxgiFactory; // We want exactly one of these since we only want to activate debug layers once

            Microsoft::WRL::ComPtr<ID3D12Device> _device;

            Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;

            static const u32 FRAME_INDEX_COUNT = 2;
            u32 _frameIndex;
            void* _fenceEvent;

            Microsoft::WRL::ComPtr<ID3D12Fence> _fences[FRAME_INDEX_COUNT];
            u64 _fenceValues[FRAME_INDEX_COUNT];

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _mainDescriptorHeap[FRAME_INDEX_COUNT];

            std::vector<ConstantBufferBackend*> _constantBufferBackends;
            std::vector<SwapChainDX12*> _swapChains;

            // We friend class handlers that need access to private variables they'll need to initialize stuff
            friend class ImageHandlerDX12;
            friend class ModelHandlerDX12;
            friend class PipelineHandlerDX12;
            friend class CommandListHandlerDX12;
        };
    }
}