#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"

namespace Renderer
{
    enum DepthImageFormat
    {
        DEPTH_IMAGE_FORMAT_UNKNOWN,
        DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT,
        DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS,
        DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT,
        DEPTH_IMAGE_FORMAT_R32_TYPELESS,
        DEPTH_IMAGE_FORMAT_D32_FLOAT,
        DEPTH_IMAGE_FORMAT_R32_FLOAT,
        DEPTH_IMAGE_FORMAT_R32_UINT,
        DEPTH_IMAGE_FORMAT_R32_SINT,
        DEPTH_IMAGE_FORMAT_R24G8_TYPELESS,
        DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT,
        DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS,
        DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT
    };

    struct DepthImageDesc
    {
        Vector2i dimensions = Vector2i(0,0);
        DepthImageFormat format = DEPTH_IMAGE_FORMAT_UNKNOWN;
        SampleCount sampleCount = SAMPLE_COUNT_1;
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(DepthImageID, u16);
}