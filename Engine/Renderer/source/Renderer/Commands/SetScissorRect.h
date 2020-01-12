#pragma once
#include <Core.h>
#include "../RenderStates.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetScissorRect
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ScissorRect scissorRect;
        };
    }
}