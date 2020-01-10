#pragma once
#include "Core.h"
#include "Descriptors/CommandListDesc.h"

namespace Renderer
{
    class Renderer;
    typedef void (*BackendDispatchFunction)(Renderer*, CommandListID, const void*);

    namespace BackendDispatch
    {
        void ClearImage(Renderer* renderer, CommandListID commandList, const void* data);
        void ClearDepthImage(Renderer* renderer, CommandListID commandList, const void* data);

        void Draw(Renderer* renderer, CommandListID commandList, const void* data);

        void PopMarker(Renderer* renderer, CommandListID commandList, const void* data);
        void PushMarker(Renderer* renderer, CommandListID commandList, const void* data);
        
        void SetGraphicsPipeline(Renderer* renderer, CommandListID commandList, const void* data);
        void SetComputePipeline(Renderer* renderer, CommandListID commandList, const void* data);
    }
}