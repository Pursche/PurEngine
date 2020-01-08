#include "RenderPassBuilder.h"
#include "RenderGraph.h"

namespace Renderer
{

    void RenderPassBuilder::Compile(CommandList* /*commandList*/)
    {

    }

    ImageID RenderPassBuilder::Create(ImageDesc& /*desc*/)
    {
        return ImageID::Invalid();
    }

    DepthImageID RenderPassBuilder::Create(DepthImageDesc& /*desc*/)
    {
        return DepthImageID::Invalid();
    }

    RenderPassResource RenderPassBuilder::Read(ImageID id, ShaderStage /*shaderStage*/)
    {
        RenderPassResource resource = _renderGraph->GetResource(id);

        return resource;
    }

    RenderPassResource RenderPassBuilder::Read(DepthImageID id, ShaderStage /*shaderStage*/)
    {
        RenderPassResource resource = _renderGraph->GetResource(id);

        return resource;
    }

    RenderPassMutableResource RenderPassBuilder::Write(ImageID id, WriteMode /*writeMode*/, LoadMode /*loadMode*/)
    {
        RenderPassMutableResource resource = _renderGraph->GetMutableResource(id);

        return resource;
    }

    RenderPassMutableResource RenderPassBuilder::Write(DepthImageID id, WriteMode /*writeMode*/, LoadMode /*loadMode*/)
    {
        RenderPassMutableResource resource = _renderGraph->GetMutableResource(id);

        return resource;
    }

    
}