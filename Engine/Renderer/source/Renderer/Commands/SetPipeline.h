#pragma once
#include <Core.h>
#include "../Descriptors/GraphicsPipelineDesc.h"
#include "../Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetGraphicsPipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            GraphicsPipelineID pipeline = GraphicsPipelineID::Invalid();
        };
        
        struct SetComputePipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ComputePipelineID pipeline = ComputePipelineID::Invalid();
        };
    }
}