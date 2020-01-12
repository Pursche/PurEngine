#pragma once
#include <Core.h>
#include "../ConstantBuffer.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetConstantBuffer
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            u32 slot = 0;
            void* gpuResource = nullptr;
        };
    }
}