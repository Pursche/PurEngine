#pragma once
#include <Core.h>

namespace Renderer
{
    // This struct needs to be 256 bytes padded
    struct InstanceData
    {
        Vector4 colorMultiplier = Vector4(1,1,1,1); // 16 bytes
        Matrix modelMatrix; // 64 bytes
        float padding[176];
    };
}