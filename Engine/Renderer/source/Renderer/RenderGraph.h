#pragma once
#include <Core.h>
#include "Descriptors/RenderGraphDesc.h"
#include "RenderPass.h"

namespace Renderer
{
    class Renderer;
    class RenderGraphBuilder;

    // Acyclic Graph for rendering
    class RenderGraph
    {
    public:

        template <typename PassData>
        void AddPass(std::string name, std::function<bool(PassData&, RenderGraphBuilder&)> onSetup, std::function<void(PassData&, CommandList&)> onExecute)
        {
            IRenderPass* pass = new RenderPass<PassData>(name, onSetup, onExecute);
            _passes.push_back(pass);
        }

        void Setup();
        void Execute();

        RenderGraphBuilder* GetBuilder() { return _renderGraphBuilder; }

    private:
        RenderGraph()
            : _renderGraphBuilder(nullptr)
        {
        
        } // This gets friend-created by Renderer
        bool Init(RenderGraphDesc& desc);

    private:
        std::vector<IRenderPass*> _passes;
        std::vector<IRenderPass*> _executingPasses;

        RenderGraphBuilder* _renderGraphBuilder;

        friend class Renderer; // To have access to the constructor
    };
}