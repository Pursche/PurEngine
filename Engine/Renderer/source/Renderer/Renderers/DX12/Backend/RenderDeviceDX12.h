#pragma once
#include <Core.h>
#include "d3dx12.h"
#include <dxgi1_4.h>

#include "../../../Descriptors/ImageDesc.h"
#include "../../../Descriptors/DepthImageDesc.h"

class Window;

namespace Renderer
{
    namespace Backend
    {
        struct ConstantBufferBackend;
        
        class ImageHandlerDX12;

        class RenderDeviceDX12
        {
        public:
            void Init();
            void InitWindow(Window*);

            ID3D12GraphicsCommandList* BeginResourceCommandList();
            void EndCommandList(ID3D12GraphicsCommandList* commandList);

            ConstantBufferBackend* CreateConstantBufferBackend(size_t size);

            ImageID CreateImage(const ImageDesc& desc);
            DepthImageID CreateDepthImage(const DepthImageDesc& desc);

        private:
            void InitOnce();
            static IDXGIFactory4* _dxgiFactory; // We want exactly one of these since we only want to activate debug layers once

            ID3D12Device* _device;

            ID3D12CommandQueue* _commandQueue;

            static const u32 FRAME_BUFFER_COUNT = 3; // TODO: Don't hardcode this
            u32 _frameIndex;

            ID3D12CommandAllocator* _commandAllocators[FRAME_BUFFER_COUNT];
            ID3D12CommandAllocator* _resourceCommandAllocators[FRAME_BUFFER_COUNT];

            ID3D12GraphicsCommandList* _commandList;
            ID3D12GraphicsCommandList* _resourceCommandList;

            ID3D12Fence* _fences[FRAME_BUFFER_COUNT];
            u64 _fenceValues[FRAME_BUFFER_COUNT];

            ID3D12DescriptorHeap* _mainDescriptorHeap[FRAME_BUFFER_COUNT];

            // Handles
            ImageHandlerDX12* _imageHandler = nullptr;

            // We friend class all handlers so they can access the private variables they'll need to initialize stuff
            friend class ImageHandlerDX12;
        };
    }
}