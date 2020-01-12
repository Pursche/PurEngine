#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"

namespace Renderer
{
    enum DepthImageFormat
    {
        DEPTH_IMAGE_FORMAT_UNKNOWN,

        // 32-bit Z w/ Stencil
        DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS,
        DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT,
        DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS,
        DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT,

        // No Stencil
        DEPTH_IMAGE_FORMAT_R32_TYPELESS,
        DEPTH_IMAGE_FORMAT_D32_FLOAT,
        DEPTH_IMAGE_FORMAT_R32_FLOAT,

        // 24-bit Z
        DEPTH_IMAGE_FORMAT_R24G8_TYPELESS,
        DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT,
        DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS,
        DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT,

        // 16-bit Z w/o Stencil
        DEPTH_IMAGE_FORMAT_R16_TYPELESS,
        DEPTH_IMAGE_FORMAT_D16_UNORM,
        DEPTH_IMAGE_FORMAT_R16_UNORM
    };

    struct DepthImageDesc
    {
        std::string debugName = "";
        Vector2i dimensions = Vector2i(0,0);
        DepthImageFormat format = DEPTH_IMAGE_FORMAT_UNKNOWN;
        SampleCount sampleCount = SAMPLE_COUNT_1;
        f32 depthClearValue = 0.0f;
        u8 stencilClearValue = 0;
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(DepthImageID, u16);
}