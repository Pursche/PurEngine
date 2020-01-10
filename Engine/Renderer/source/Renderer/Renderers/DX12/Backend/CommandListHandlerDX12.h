#pragma once
#include <Core.h>
#include <vector>
#include <queue>
#include "d3dx12.h"

#include "../../../Descriptors/CommandListDesc.h"

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;

        class CommandListHandlerDX12
        {
        public:
            CommandListHandlerDX12();
            ~CommandListHandlerDX12();

            CommandListID BeginCommandList(RenderDeviceDX12* device);
            void EndCommandList(RenderDeviceDX12* device, CommandListID id);

            ID3D12GraphicsCommandList* GetCommandList(CommandListID id);

        private:
            struct CommandList
            {
                ID3D12GraphicsCommandList* commandList;
                ID3D12CommandAllocator* allocator;
            };

            CommandListID CreateCommandList(RenderDeviceDX12* device);

        private:
            
        private:
            std::vector<CommandList> _commandLists;
            std::queue<CommandListID> _availableCommandLists;
        };
    }
}