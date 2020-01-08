#pragma once
#include <Core.h>
#include <vector>
#include <functional>

#include "RenderStates.h"
#include "RenderPassResources.h"

#include "Descriptors/ImageDesc.h"
#include "Descriptors/DepthImageDesc.h"

namespace Renderer
{
    class CommandList;
    class RenderGraph;

    class RenderPassBuilder
    {
    public:
        enum WriteMode
        {
            WRITE_MODE_RENDERTARGET,
            WRITE_MODE_UAV
        };

        enum LoadMode
        {
            LOAD_MODE_LOAD, // Load the contents of the resource
            LOAD_MODE_DISCARD, // Load the contents of the resource, but we don't really care
            LOAD_MODE_CLEAR // Clear the resource of existing data
        };

        enum ShaderStage
        {
            SHADER_STAGE_NONE = 0,
            SHADER_STAGE_VERTEX = 1,
            SHADER_STAGE_PIXEL = 2,
            SHADER_STAGE_COMPUTE = 4
        };

        // Create transient resources
        ImageID Create(ImageDesc& desc);
        DepthImageID Create(DepthImageDesc& desc);

        // Reads
        RenderPassResource Read(ImageID id, ShaderStage shaderStage);
        RenderPassResource Read(DepthImageID id, ShaderStage shaderStage);

        // Writes
        RenderPassMutableResource Write(ImageID id, WriteMode writeMode, LoadMode loadMode);
        RenderPassMutableResource Write(DepthImageID id, WriteMode writeMode, LoadMode loadMode);

        // Render states
        void SetRasterizerState(RasterizerState& rasterizerState) { _rasterizerState = rasterizerState; }
        void SetDepthStencilState(DepthStencilState& depthStencilState) { _depthStencilState = depthStencilState; }

    private:
        void Compile(CommandList* commandList);

        RenderPassBuilder(RenderGraph* renderGraph)
            : _renderGraph(renderGraph)
        {

        }

    private:
        RasterizerState _rasterizerState;
        DepthStencilState _depthStencilState;
        RenderGraph* _renderGraph;

        template<typename T> friend class RenderPass;
    };
}