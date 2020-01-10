#include "CommandListHandlerDX12.h"
#include "RenderDeviceDX12.h"
#include "d3dx12.h"
#include <Utils/StringUtils.h>
#include <cassert>

namespace Renderer
{
    namespace Backend
    {
        CommandListHandlerDX12::CommandListHandlerDX12()
        {

        }

        CommandListHandlerDX12::~CommandListHandlerDX12()
        {

        }

        CommandListID CommandListHandlerDX12::BeginCommandList(RenderDeviceDX12* device)
        {
            using type = type_safe::underlying_type<CommandListID>;

            CommandListID id;
            if (_availableCommandLists.size() > 0)
            {
                id = _availableCommandLists.front();
                _availableCommandLists.pop();

                CommandList& commandList = _commandLists[static_cast<type>(id)];
                HRESULT result = commandList.commandList->Reset(commandList.allocator, NULL);
                assert(SUCCEEDED(result)); // We failed to reset the commandlist
            }
            else
            {
                return CreateCommandList(device);
            }

            return id;
        }

        void CommandListHandlerDX12::EndCommandList(RenderDeviceDX12* device, CommandListID id)
        {
            using type = type_safe::underlying_type<CommandListID>;
            CommandList& commandList = _commandLists[static_cast<type>(id)];

            // Execute command list
            commandList.commandList->Close();
            ID3D12CommandList* ppCommandLists[] = { commandList.commandList };
            device->_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            u32 frameIndex = device->GetFrameIndex();
            // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
            device->_fenceValues[frameIndex]++;
            HRESULT result = device->_commandQueue->Signal(device->_fences[frameIndex], device->_fenceValues[frameIndex]);
            assert(SUCCEEDED(result)); // Failed to signal fence

            _availableCommandLists.push(id);
        }

        ID3D12GraphicsCommandList* CommandListHandlerDX12::GetCommandList(CommandListID id)
        {
            using type = type_safe::underlying_type<CommandListID>;
            CommandList& commandList = _commandLists[static_cast<type>(id)];

            return commandList.commandList;
        }

        CommandListID CommandListHandlerDX12::CreateCommandList(RenderDeviceDX12* device)
        {
            size_t id = _commandLists.size();
            assert(id < CommandListID::MaxValue());
            using type = type_safe::underlying_type<CommandListID>;

            HRESULT result;

            CommandList commandList;

            // Create the Command Allocator
            result = device->_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandList.allocator));
            assert(SUCCEEDED(result)); // Failed to create command list allocator
            
            // Create the command list with the first allocator
            result = device->_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandList.allocator, NULL, IID_PPV_ARGS(&commandList.commandList));
            assert(SUCCEEDED(result)); // Failed to create command list

            return CommandListID(static_cast<type>(id));
        }
    }
}