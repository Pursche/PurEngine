#pragma once
#include <Core.h>
#include <functional>
#include <algorithm>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"
#include "../RenderPassResources.h"

#include "ImageDesc.h"
#include "DepthImageDesc.h"

namespace Renderer
{
    struct MaterialDesc
    {
        std::string path;
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(MaterialID, u16);

    struct MaterialPipelineDesc
    {
        MaterialPipelineDesc()
        {
            std::fill_n(renderTargets, MAX_RENDER_TARGETS, RenderPassMutableResource::Invalid());
        }

        static const int MAX_BOUND_TEXTURES = 8;

        MaterialID material;

        struct States
        {
            RasterizerState rasterizerState;
            DepthStencilState depthStencilState;
        };
        States states;

        // Functions for converting resources to ImageIDs
        std::function<DepthImageID(RenderPassResource resource)> ResourceToDepthImageID = nullptr;
        std::function<ImageID(RenderPassMutableResource resource)> MutableResourceToImageID = nullptr;
        std::function<DepthImageID(RenderPassMutableResource resource)> MutableResourceToDepthImageID = nullptr;

        // Rendertargets
        RenderPassMutableResource renderTargets[MAX_RENDER_TARGETS];
        RenderPassMutableResource depthStencil = RenderPassMutableResource::Invalid();
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(MaterialPipelineID, u16);
}