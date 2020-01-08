#pragma once
#include <Core.h>
#include "ICommand.h"
#include "../Descriptors/ImageDesc.h"
#include "../Descriptors/DepthImageDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct ColorClearCommand : ICommand
        {
            ImageID imageID;
            Vector3 color;
        };

        struct DepthClearCommand : ICommand
        {
            DepthImageID imageID;
            Vector3 color;
        };
    }
}