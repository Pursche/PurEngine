#pragma once
#include <Core.h>
#include <vector>
#include <functional>

#include "CommandList.h"
#include "Descriptors/GraphicsPipelineDesc.h"
#include "Descriptors/ComputePipelineDesc.h"

namespace Renderer
{
    class Renderer;
    class RenderLayer;
    class RenderGraph;
    class RenderGraphBuilder;

    class IRenderPass
    {
    public:
        virtual bool Setup(RenderGraphBuilder* renderGraphBuilder) = 0;
        virtual void Execute() = 0;
    };

    template <typename PassData>
    class RenderPass : public IRenderPass
    {
        typedef std::function<bool(PassData&, RenderGraphBuilder&)> SetupFunction;
        typedef std::function<void(PassData&, CommandList&)> ExecuteFunction;

    private:
        RenderPass(std::string name, SetupFunction onSetup, ExecuteFunction onExecute, Renderer* renderer)
            : _commandList(renderer)
        {
            _name = name;
            _onSetup = onSetup;
            _onExecute = onExecute;
        }

        bool Setup(RenderGraphBuilder* renderGraphBuilder) override
        {
            return _onSetup(_data, *renderGraphBuilder);
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
        CommandList _commandList;

        GraphicsPipelineID _graphicsPipeline = GraphicsPipelineID::Invalid();
        ComputePipelineID _computePipeline = ComputePipelineID::Invalid();
        std::vector<RenderLayer*> _renderLayers;

        friend class RenderGraph;
    };
}