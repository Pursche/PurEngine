#pragma once
#include "CommandList.h"

namespace Renderer
{
    void CommandList::SetPipeline(GraphicsPipelineID pipelineID)
    {
        Commands::SetGPipelineCommand* command = new Commands::SetGPipelineCommand();
        command->pipelineID = pipelineID;
        _commands.push_back(command);
    }

    void CommandList::SetPipeline(ComputePipelineID pipelineID)
    {
        Commands::SetCPipelineCommand* command = new Commands::SetCPipelineCommand();
        command->pipelineID = pipelineID;
        _commands.push_back(command);
    }

    void CommandList::Draw(const ModelID modelID, const InstanceData& instance)
    {
        Commands::DrawCommand* command = new Commands::DrawCommand(modelID, instance);
        _commands.push_back(command);
    }

    void CommandList::Clear(ImageID imageID, Vector3 color)
    {
        Commands::ColorClearCommand* command = new Commands::ColorClearCommand();
        command->imageID = imageID;
        command->color = color;
        _commands.push_back(command);
    }

    void CommandList::Clear(DepthImageID imageID, Vector3 color)
    {
        Commands::DepthClearCommand* command = new Commands::DepthClearCommand();
        command->imageID = imageID;
        command->color = color;
        _commands.push_back(command);
    }

    void CommandList::PushMarker(std::string marker)
    {
        Commands::PushMarkerCommand* command = new Commands::PushMarkerCommand();
        command->marker = marker;
        _commands.push_back(command);
    }

    void CommandList::PopMarker()
    {
        _commands.push_back(new Commands::PopMarkerCommand());
    }
}