#pragma once
#include "CommandList.h"
#include "Renderer.h"

namespace Renderer
{
    void CommandList::Execute()
    {
        CommandListID commandList = _renderer->BeginCommandList();

        // Execute each command
        for (int i = 0; i < _functions.Count(); i++)
        {
            _functions[i](_renderer, commandList, _data[i]);
        }

        _renderer->EndCommandList(commandList);
    }
}