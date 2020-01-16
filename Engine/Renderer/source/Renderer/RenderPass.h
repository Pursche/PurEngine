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
        virtual void Execute(CommandList& commandList) = 0;
    };

    template <typename PassData>
    class RenderPass : public IRenderPass
    {
    public:
        typedef std::function<bool(PassData&, RenderGraphBuilder&)> SetupFunction;
        typedef std::function<void(PassData&, CommandList&)> ExecuteFunction;
    
        RenderPass(std::string& name, SetupFunction onSetup, ExecuteFunction onExecute)
            : _name(name)
            , _onSetup(onSetup)
            , _onExecute(onExecute)
        {

        }

    private:
        bool Setup(RenderGraphBuilder* renderGraphBuilder) override
        {
            return _onSetup(_data, *renderGraphBuilder);
        }

        void Execute(CommandList& commandList) override
        {
            _onExecute(_data, commandList);
        }

        bool ShouldRun() { return _shouldRun; }
    private:

    private:
        std::string& _name;
        SetupFunction _onSetup;
        ExecuteFunction _onExecute;

        PassData _data;
    };
}