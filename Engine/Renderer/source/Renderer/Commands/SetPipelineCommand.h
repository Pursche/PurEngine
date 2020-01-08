#pragma once
#include <Core.h>
#include "ICommand.h"
#include "../Descriptors/GraphicsPipelineDesc.h"
#include "../Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetGPipelineCommand : ICommand
        {
            GraphicsPipelineID pipelineID;
        };

        struct SetCPipelineCommand : ICommand
        {
            ComputePipelineID pipelineID;
        };
    }
}