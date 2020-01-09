#pragma once
#include <Core.h>
#include "ICommand.h"
#include "../Descriptors/ModelDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct PushMarkerCommand : ICommand
        {
            std::string marker;
        };
    }
}