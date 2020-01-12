#pragma once
#include <Core.h>
#include "../Descriptors/ModelDesc.h"
#include "../InstanceData.h"

namespace Renderer
{
    namespace Commands
    {
        struct Draw
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ModelID model = ModelID::Invalid();
        };
    }
}