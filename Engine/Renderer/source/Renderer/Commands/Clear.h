#pragma once
#include <Core.h>
#include "../BackendDispatch.h"
#include "../Descriptors/ImageDesc.h"
#include "../Descriptors/DepthImageDesc.h"

namespace Renderer
{
    namespace Commands
    {
        struct ClearImage
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            ImageID image;
            Vector4 color;
        };
        const BackendDispatchFunction ClearImage::DISPATCH_FUNCTION = &BackendDispatch::ClearImage;

        struct ClearDepthImage
        {
            static const BackendDispatchFunction DISPATCH_FUNCTION;

            DepthImageID image;
            DepthClearFlags flags;
            f32 depth;
            u8 stencil;
        };
        const BackendDispatchFunction ClearDepthImage::DISPATCH_FUNCTION = &BackendDispatch::ClearDepthImage;
    }
}