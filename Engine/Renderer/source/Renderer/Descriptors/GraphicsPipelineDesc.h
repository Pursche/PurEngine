#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include "../RenderStates.h"
#include "../RenderPassResources.h"

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
        BlendState blendStates[MAX_RENDER_TARGETS];

        // Rendertargets
        RenderPassMutableResource renderTargets[MAX_RENDER_TARGETS]{ RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid(), RenderPassMutableResource::Invalid() };
        RenderPassMutableResource depthStencil = RenderPassMutableResource::Invalid();

        // Shaders
        VertexShaderID vertexShader = VertexShaderID::Invalid();
        PixelShaderID pixelShader = PixelShaderID::Invalid();
    };

    // Lets strong-typedef an ID type with the underlying type of u16
    STRONG_TYPEDEF(GraphicsPipelineID, u16);
}