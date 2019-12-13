#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"

#include "VertexShaderDesc.h"
#include "PixelShaderDesc.h"
#include "ImageDesc.h"
#include "DepthImageDesc.h"
#include "RenderTargetDesc.h"

namespace Renderer
{
    struct GraphicsPipelineDesc
    {
        static const int MAX_RENDER_TARGETS = 8;

        // States
        RasterizerState rasterizerState;
        DepthStencilState depthStencilState;

        // Rendertargets
        RenderTargetDesc renderTargets[MAX_RENDER_TARGETS];
        DepthImageID depthStencil = DepthImageID::Invalid();

        // Shaders
        VertexShaderID vertexShader = VertexShaderID::Invalid();
        PixelShaderID pixelShader = PixelShaderID::Invalid();
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(GraphicsPipelineID, u16);
}