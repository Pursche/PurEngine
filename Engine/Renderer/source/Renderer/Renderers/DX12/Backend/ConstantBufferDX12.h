#pragma once
#include <Core.h>
#include "../../../ConstantBuffer.h"
#include "d3dx12.h"

namespace Renderer
{
    namespace Backend
    {
        struct ConstantBufferBackendDX12 : public ConstantBufferBackend
        {
            ConstantBufferBackendDX12() {}; // Allow creation of this

            static const u32 FRAME_BUFFER_COUNT = 3; // TODO: Don't hardcode this

            ID3D12Resource* uploadHeap[FRAME_BUFFER_COUNT] = {};
            void* gpuAddress[FRAME_BUFFER_COUNT] = {};

        private:
            void Apply(u32 frameIndex, void* data, size_t size) override
            {
                memcpy(gpuAddress[frameIndex], data, size);
            }

            void* GetGPUResource(u32 frameIndex) override
            {
                return uploadHeap[frameIndex];
            }
        };
    }
}