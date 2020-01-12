#pragma once
#include "Core.h"
#include "Descriptors/CommandListDesc.h"

namespace Renderer
{
    class Renderer;
    typedef void (*BackendDispatchFunction)(Renderer*, CommandListID, const void*);

    class BackendDispatch
    {
    public:
        static void ClearImage(Renderer* renderer, CommandListID commandList, const void* data);
        static void ClearDepthImage(Renderer* renderer, CommandListID commandList, const void* data);

        static void Draw(Renderer* renderer, CommandListID commandList, const void* data);

        static void PopMarker(Renderer* renderer, CommandListID commandList, const void* data);
        static void PushMarker(Renderer* renderer, CommandListID commandList, const void* data);

        static void SetConstantBuffer(Renderer* renderer, CommandListID commandList, const void* data);
        static void SetGraphicsPipeline(Renderer* renderer, CommandListID commandList, const void* data);
        static void SetComputePipeline(Renderer* renderer, CommandListID commandList, const void* data);

        static void SetScissorRect(Renderer* renderer, CommandListID commandList, const void* data);
        static void SetViewport(Renderer* renderer, CommandListID commandList, const void* data);

        static void PresentImage(Renderer* renderer, CommandListID commandList, const void* data);
        static void PresentDepthImage(Renderer* renderer, CommandListID commandList, const void* data);
    };
}