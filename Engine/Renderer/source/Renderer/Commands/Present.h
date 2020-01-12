#pragma once
#include <Core.h>
#include "../Descriptors/ImageDesc.h"
#include "../Descriptors/DepthImageDesc.h"

class Window;

namespace Renderer
{
    namespace Commands
    {
        struct PresentImage
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            Window* window = nullptr;
            ImageID image = ImageID::Invalid();
        };
        
        struct PresentDepthImage
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            Window* window = nullptr;
            DepthImageID image = DepthImageID::Invalid();
        };
    }
}