#include "BackendDispatch.h"
#include "Renderer.h"

#include "Commands/Clear.h"
#include "Commands/Draw.h"
#include "Commands/PopMarker.h"
#include "Commands/PushMarker.h"
#include "Commands/SetPipeline.h"
#include "Commands/SetScissorRect.h"
#include "Commands/SetViewport.h"

namespace Renderer
{
    void BackendDispatch::ClearImage(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::ClearImage* actualData = static_cast<const Commands::ClearImage*>(data);
        renderer->Clear(commandList, actualData->image, actualData->color);
    }
    void BackendDispatch::ClearDepthImage(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::ClearDepthImage* actualData = static_cast<const Commands::ClearDepthImage*>(data);
        renderer->Clear(commandList, actualData->image, actualData->flags, actualData->depth, actualData->stencil);
    }

    void BackendDispatch::Draw(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::Draw* actualData = static_cast<const Commands::Draw*>(data);
        renderer->Draw(commandList, actualData->model);
    }

    void BackendDispatch::PopMarker(Renderer* renderer, CommandListID commandList, const void* /*data*/)
    {
        renderer->PopMarker(commandList);
    }

    void BackendDispatch::PushMarker(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::PushMarker* actualData = static_cast<const Commands::PushMarker*>(data);
        renderer->PushMarker(commandList, actualData->color, actualData->marker);
    }

    void BackendDispatch::SetConstantBuffer(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetConstantBuffer* actualData = static_cast<const Commands::SetConstantBuffer*>(data);
        renderer->SetConstantBuffer(commandList, actualData->slot, actualData->gpuResource);
    }

    void BackendDispatch::SetGraphicsPipeline(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetGraphicsPipeline* actualData = static_cast<const Commands::SetGraphicsPipeline*>(data);
        renderer->SetPipeline(commandList, actualData->pipeline);
    }

    void BackendDispatch::SetMaterialPipeline(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetMaterialPipeline* actualData = static_cast<const Commands::SetMaterialPipeline*>(data);
        renderer->SetPipeline(commandList, actualData->pipeline);
    }

    void BackendDispatch::SetComputePipeline(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetComputePipeline* actualData = static_cast<const Commands::SetComputePipeline*>(data);
        renderer->SetPipeline(commandList, actualData->pipeline);
    }

    void BackendDispatch::SetScissorRect(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetScissorRect* actualData = static_cast<const Commands::SetScissorRect*>(data);
        renderer->SetScissorRect(commandList, actualData->scissorRect);
    }

    void BackendDispatch::SetViewport(Renderer* renderer, CommandListID commandList, const void* data)
    {
        const Commands::SetViewport* actualData = static_cast<const Commands::SetViewport*>(data);
        renderer->SetViewport(commandList, actualData->viewport);
    }
}