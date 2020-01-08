#pragma once
#include <Core.h>
#include "Commands/SetPipelineCommand.h"
#include "Commands/DrawCommand.h"
#include "Commands/ClearCommand.h"
#include "Commands/PushMarkerCommand.h"
#include "Commands/PopMarkerCommand.h"
#include <vector>

namespace Renderer
{
    class CommandList
    {
    public:
        void SetPipeline(GraphicsPipelineID pipelineID);
        void SetPipeline(ComputePipelineID pipelineID);

        void Draw(const ModelID modelID, const InstanceData& instance);
        void Clear(ImageID imageID, Vector3 color);
        void Clear(DepthImageID imageID, Vector3 color);

        void PushMarker(std::string marker);
        void PopMarker();

    private:

    private:
        std::vector<Commands::ICommand*> _commands; // TODO: Figure out a better way to store these
    };

    class ScopedMarker
    {
    public:
        ScopedMarker(CommandList& commandList, std::string marker)
            : _commandList(commandList)
        {
            _commandList.PushMarker(marker);
        }
        ~ScopedMarker()
        {
            _commandList.PopMarker();
        }

    private:
        CommandList& _commandList;
    };
}