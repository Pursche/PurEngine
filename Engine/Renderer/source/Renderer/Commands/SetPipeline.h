#pragma once
#include <Core.h>
#include "../Descriptors/GraphicsPipelineDesc.h"
#include "../Descriptors/ComputePipelineDesc.h"
#include "../Descriptors/MaterialDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetGraphicsPipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            GraphicsPipelineID pipeline = GraphicsPipelineID::Invalid();
        };

        struct SetMaterialPipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            MaterialPipelineID pipeline = MaterialPipelineID::Invalid();
        };
        
        struct SetComputePipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ComputePipelineID pipeline = ComputePipelineID::Invalid();
        };
    }
}