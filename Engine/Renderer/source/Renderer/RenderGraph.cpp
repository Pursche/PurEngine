#include "RenderGraph.h"

namespace Renderer
{
    bool RenderGraph::Init(RenderGraphDesc& /*desc*/)
    {

        return true;
    }

    /*void RenderGraph::AddPass(RenderPass& pass)
    {
        _passes.push_back(pass);
    }*/

    void RenderGraph::Setup()
    {
        for (IRenderPass* pass : _passes)
        {
            if (pass->Setup())
            {
                _executingPasses.push_back(pass);
            }
        }
    }

    void RenderGraph::Execute()
    {
        // TODO: Parallel_for this
        for (IRenderPass* pass : _executingPasses)
        {
            pass->Execute();
        }

        // TODO: Merge all renderpass commandlists into one and execute
    }

    RenderPassResource RenderGraph::GetResource(ImageID id)
    {
        using _type = type_safe::underlying_type<ImageID>;

        _type i = 0;
        for(ImageID& trackedID : _trackedImages)
        {
            if (trackedID == id)
            {
                return RenderPassResource(i);
            }

            i++;
        }

        _trackedImages.push_back(id);
        return RenderPassResource(i);
    }

    RenderPassResource RenderGraph::GetResource(DepthImageID id)
    {
        using _type = type_safe::underlying_type<DepthImageID>;

        _type i = 0;
        for (DepthImageID& trackedID : _trackedDepthImages)
        {
            if (trackedID == id)
            {
                return RenderPassResource(i);
            }

            i++;
        }

        _trackedDepthImages.push_back(id);
        return RenderPassResource(i);
    }

    RenderPassMutableResource RenderGraph::GetMutableResource(ImageID id)
    {
        using _type = type_safe::underlying_type<ImageID>;

        _type i = 0;
        for (ImageID& trackedID : _trackedImages)
        {
            if (trackedID == id)
            {
                return RenderPassMutableResource(i);
            }

            i++;
        }

        _trackedImages.push_back(id);
        return RenderPassMutableResource(i);
    }

    RenderPassMutableResource RenderGraph::GetMutableResource(DepthImageID id)
    {
        using _type = type_safe::underlying_type<DepthImageID>;

        _type i = 0;
        for (DepthImageID& trackedID : _trackedDepthImages)
        {
            if (trackedID == id)
            {
                return RenderPassMutableResource(i);
            }

            i++;
        }

        _trackedDepthImages.push_back(id);
        return RenderPassMutableResource(i);
    }


}