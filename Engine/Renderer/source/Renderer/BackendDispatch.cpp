#include "BackendDispatch.h"
#include "Renderer.h"

#include "Commands/Clear.h"
#include "Commands/Draw.h"
#include "Commands/PopMarker.h"
#include "Commands/PushMarker.h"
#include "Commands/SetPipeline.h"

namespace Renderer
{
    namespace BackendDispatch
    {
        void ClearImage(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::ClearImage* actualData = static_cast<const Commands::ClearImage*>(data);
            renderer->Clear(commandList, actualData->image, actualData->color);
        }
        void ClearDepthImage(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::ClearDepthImage* actualData = static_cast<const Commands::ClearDepthImage*>(data);
            renderer->Clear(commandList, actualData->image, actualData->flags, actualData->depth, actualData->stencil);
        }

        void Draw(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::Draw* actualData = static_cast<const Commands::Draw*>(data);
            renderer->Draw(commandList, actualData->model);
        }

        void PopMarker(Renderer* renderer, CommandListID commandList, const void* /*data*/)
        {
            renderer->PopMarker(commandList);
        }

        void PushMarker(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::PushMarker* actualData = static_cast<const Commands::PushMarker*>(data);
            renderer->PushMarker(commandList, actualData->color, actualData->marker);
        }

        void SetGraphicsPipeline(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::SetGraphicsPipeline* actualData = static_cast<const Commands::SetGraphicsPipeline*>(data);
            renderer->SetPipeline(commandList, actualData->pipeline);
        }

        void SetComputePipeline(Renderer* renderer, CommandListID commandList, const void* data)
        {
            const Commands::SetComputePipeline* actualData = static_cast<const Commands::SetComputePipeline*>(data);
            renderer->SetPipeline(commandList, actualData->pipeline);
        }
    }
}