#pragma once
#include <Core.h>
#include "ConstantBuffer.h"

namespace Renderer
{
    class Renderer;

    struct ModelConstantBuffer
    {
        Vector4 colorMultiplier; // 16 bytes
        Matrix modelMatrix; // 64 bytes

        float padding[128] = {};
    };

    // This struct needs to be 256 bytes padded
    struct InstanceData
    {
        InstanceData(Renderer* renderer);

        Vector4 colorMultiplier = Vector4(1, 1, 1, 1);
        Vector3 position = Vector3(0, 0, 0);
        Vector3 rotation = Vector3(0, 0, 0);
        Vector3 scale = Vector3(1, 1, 1);

        void Apply(u32 frameIndex);
        void* GetGPUResource(u32 frameIndex);

    private:
        ConstantBuffer<ModelConstantBuffer> modelCB;
    };
}