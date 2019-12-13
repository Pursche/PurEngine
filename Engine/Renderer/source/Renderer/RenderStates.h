#pragma once
#include <Core.h>

namespace Renderer
{
    enum SampleCount
    {
        SAMPLE_COUNT_1,
        SAMPLE_COUNT_2,
        SAMPLE_COUNT_4,
        SAMPLE_COUNT_8
    };

    enum CullState
    {
        CULL_STATE_NONE,
        CULL_STATE_FRONT,
        CULL_STATE_BACK
    };

    enum FrontFaceState
    {
        FRONT_STATE_CLOCKWISE,
        FRONT_STATE_COUNTERCLOCKWISE
    };

    struct RasterizerState
    {
        CullState cullState;
        FrontFaceState frontFaceState;
        bool depthBiasEnabled;
        f32 depthBias;
        f32 depthBiasClamp;
        f32 depthBiasSlopeFactor;
        SampleCount sampleCount;
    };
}