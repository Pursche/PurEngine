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

        class RenderDeviceDX12
        {
        public:
            ~RenderDeviceDX12();

            void Init();
            void InitWindow(ShaderHandlerDX12* shaderHandler, CommandListHandlerDX12* commandListHandler, Window*);

            ConstantBufferBackend* CreateConstantBufferBackend(size_t size);

            u32 GetFrameIndex() { return _frameIndex; }
            void EndFrame() { _frameIndex = (_frameIndex + 1) % FRAME_INDEX_COUNT; }

        private:
            void InitOnce();
            static IDXGIFactory4* _dxgiFactory; // We want exactly one of these since we only want to activate debug layers once

            ID3D12Device* _device;

            ID3D12CommandQueue* _commandQueue;

            static const u32 FRAME_INDEX_COUNT = 2;
            u32 _frameIndex;
            void* _fenceEvent;

            ID3D12Fence* _fences[FRAME_INDEX_COUNT];
            u64 _fenceValues[FRAME_INDEX_COUNT];

            ID3D12DescriptorHeap* _mainDescriptorHeap[FRAME_INDEX_COUNT];

            // We friend class handlers that need access to private variables they'll need to initialize stuff
            friend class ImageHandlerDX12;
            friend class ModelHandlerDX12;
            friend class PipelineHandlerDX12;
            friend class CommandListHandlerDX12;
        };
    }
}