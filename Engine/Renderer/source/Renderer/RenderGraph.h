#pragma once
#include <Core.h>
#include "Descriptors/RenderGraphDesc.h"
#include "RenderPass.h"

namespace Renderer
{
    // Acyclic Graph for rendering
    class RenderGraph
    {
    public:

        template <typename PassData>
        void AddPass(std::string name, std::function<bool(PassData&, RenderPassBuilder&)> onSetup, std::function<void(PassData&, CommandList&)> onExecute)
        {
            IRenderPass* pass = new RenderPass<PassData>(this, name, onSetup, onExecute);
            _passes.push_back(pass);
        }

        void Setup();
        void Execute();

    private:
        RenderGraph(){} // This gets friend-created by Renderer
        bool Init(RenderGraphDesc& desc);

        RenderPassResource GetResource(ImageID id);
        RenderPassResource GetResource(DepthImageID id);
        RenderPassMutableResource GetMutableResource(ImageID id);
        RenderPassMutableResource GetMutableResource(DepthImageID id);

    private:
        std::vector<IRenderPass*> _passes;
        std::vector<IRenderPass*> _executingPasses;

        std::vector<ImageID> _trackedImages;
        std::vector<DepthImageID> _trackedDepthImages;

        friend class Renderer; // To have access to the constructor
        friend class RenderPassBuilder; // To have access to GetResource/GetMutableResource
    };
}