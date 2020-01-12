#pragma once
#include <Core.h>

namespace Renderer
{
    namespace Commands
    {
        struct PushMarker
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            Vector3 color = Vector3(1, 1, 1);
            std::string marker;
        };
    }
}