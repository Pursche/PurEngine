#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "ImageDesc.h"
#include "../RenderStates.h"

namespace Renderer
{
    struct RenderTargetDesc
    {
        ImageID image = ImageID::Invalid();
        BlendState blendState;
    };
}