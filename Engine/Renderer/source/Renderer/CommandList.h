#pragma once
#include <Core.h>
#include "BackendDispatch.h"
#include "Descriptors/CommandListDesc.h"
#include <vector>

// Commands
#include "Commands/Clear.h"
#include "Commands/Draw.h"
#include "Commands/PopMarker.h"
#include "Commands/PushMarker.h"
#include "Commands/SetPipeline.h"

namespace Renderer
{
    class CommandList
    {
    public:
        CommandList(Renderer* renderer)
            : _renderer(renderer)
        {

        }

        // Execute
        void Execute();

        template<typename Command>
        Command* AddCommand()
        {
            Command* command = AllocateCommand<Command>();

            AddFunction(Command::DISPATCH_FUNCTION);
            AddData(command);

            return command;
        }

    private:
        template<typename Command>
        Command* AllocateCommand()
        {
            return new Command(); // TODO: Frame allocator
        }

        void AddFunction(BackendDispatchFunction function)
        {
            _functions.push_back(function);
        }

        void AddData(void* data)
        {
            _data.push_back(data);
        }

    private:
        Renderer* _renderer;
        std::vector<BackendDispatchFunction> _functions;
        std::vector<void*> _data;
    };

    class ScopedMarker
    {
    public:
        ScopedMarker(CommandList& commandList, std::string marker)
            : _commandList(commandList)
        {
            Commands::PushMarker* command = _commandList.AddCommand<Commands::PushMarker>();
            command->marker = marker;
        }
        ~ScopedMarker()
        {
            _commandList.AddCommand<Commands::PopMarker>();
        }

    private:
        CommandList& _commandList;
    };
}