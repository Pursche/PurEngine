#pragma once
#include <Core.h>
#include "../RenderStates.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetViewport
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            Viewport viewport;
        };
    }
}