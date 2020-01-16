#pragma once
#include <Core.h>
#include "BackendDispatch.h"
#include "Descriptors/CommandListDesc.h"
#include <vector>
#include <Memory/StackAllocator.h>
#include <Containers/DynamicArray.h>

// Commands
#include "Commands/Clear.h"
#include "Commands/Draw.h"
#include "Commands/PopMarker.h"
#include "Commands/PushMarker.h"
#include "Commands/SetConstantBuffer.h"
#include "Commands/SetPipeline.h"
#include "Commands/SetScissorRect.h"
#include "Commands/SetViewport.h"
#include "Commands/Present.h"

namespace Renderer
{
    class CommandList
    {
    public:
        CommandList(Renderer* renderer, Memory::Allocator* allocator)
            : _renderer(renderer)
            , _allocator(allocator)
            , _functions(allocator, 32)
            , _data(allocator, 32)
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
            assert(_allocator != nullptr);

            return Memory::Allocator::New<Command>(_allocator);
        }

        void AddFunction(const BackendDispatchFunction& function)
        {
            _functions.Insert(function);
        }

        void AddData(void* data)
        {
            _data.Insert(data);
        }

    private:
        Memory::Allocator* _allocator;
        Renderer* _renderer;

        DynamicArray<BackendDispatchFunction> _functions;
        DynamicArray<void*> _data;
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