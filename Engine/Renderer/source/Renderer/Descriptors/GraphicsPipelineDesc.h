#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"

#include "VertexShaderDesc.h"
#include "PixelShaderDesc.h"

namespace Renderer
{
    struct GraphicsPipelineDesc
    {
        CullState cullState = CULL_STATE_FRONT;
        FrontFaceState frontFaceState = FRONT_STATE_CLOCKWISE;
        SampleCount sampleCount = SAMPLE_COUNT_1;
        VertexShaderID vertexShader = VertexShaderID::Invalid();
        PixelShaderID pixelShader = PixelShaderID::Invalid();
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(GraphicsPipelineID, u16);
}