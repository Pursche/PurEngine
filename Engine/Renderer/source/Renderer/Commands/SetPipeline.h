#pragma once
#include <Core.h>
#include "../BackendDispatch.h"
#include "../Descriptors/GraphicsPipelineDesc.h"
#include "../Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct SetGraphicsPipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            GraphicsPipelineID pipeline;
        };
        const BackendDispatchFunction SetGraphicsPipeline::DISPATCH_FUNCTION = &BackendDispatch::SetGraphicsPipeline;

        struct SetComputePipeline
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ComputePipelineID pipeline;
        };
        const BackendDispatchFunction SetComputePipeline::DISPATCH_FUNCTION = &BackendDispatch::SetComputePipeline;
    }
}