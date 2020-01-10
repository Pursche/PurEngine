#pragma once
#include <Core.h>
#include "../BackendDispatch.h"

namespace Renderer
{
    namespace Commands
    {
        struct PopMarker
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;
        };

        const BackendDispatchFunction PopMarker::DISPATCH_FUNCTION = &BackendDispatch::PopMarker;
    }
}