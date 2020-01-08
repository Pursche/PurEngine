#pragma once
#include <Core.h>
#include <vector>
#include <functional>
#include "RenderPassBuilder.h"

#include "CommandList.h"
#include "Descriptors/GraphicsPipelineDesc.h"
#include "Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    class RenderLayer;
    class RenderGraph;

    class IRenderPass
    {
    public:
        virtual bool Setup() = 0;
        virtual void Execute() = 0;
    };

    template <typename PassData>
    class RenderPass : public IRenderPass
    {
        typedef std::function<bool(PassData&, RenderPassBuilder&)> SetupFunction;
        typedef std::function<void(PassData&, CommandList&)> ExecuteFunction;

    private:
        RenderPass(RenderGraph* renderGraph, std::string name, SetupFunction onSetup, ExecuteFunction onExecute)
            : _renderPassBuilder(renderGraph)
        {
            //_renderPassBuilder.SetCommandList(&_commandList);

            _name = name;
            _onSetup = onSetup;
            _onExecute = onExecute;
        }

        bool Setup() override
        {
            return _onSetup(_data, _renderPassBuilder);
        }

        void Execute() override
        {
            _onExecute(_data, _commandList);
        }

        bool ShouldRun() { return _shouldRun; }
    private:

    private:
        std::string _name;
        SetupFunction _onSetup;
        ExecuteFunction _onExecute;

        PassData _data;
        RenderPassBuilder _renderPassBuilder;
        CommandList _commandList;

        GraphicsPipelineID _graphicsPipeline = GraphicsPipelineID::Invalid();
        ComputePipelineID _computePipeline = ComputePipelineID::Invalid();
        std::vector<RenderLayer*> _renderLayers;

        friend class RenderGraph;
    };
}