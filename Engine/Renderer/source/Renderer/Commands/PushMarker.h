#pragma once
#include <Core.h>
#include "../BackendDispatch.h"

namespace Renderer
{
    namespace Commands
    {
        struct PushMarker
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            Vector3 color;
            std::string marker;
        };

        const BackendDispatchFunction PushMarker::DISPATCH_FUNCTION = &BackendDispatch::PushMarker;
    }
}