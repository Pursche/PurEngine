#pragma once
#include <Core.h>
#include "ICommand.h"
#include "../Descriptors/ModelDesc.h"
#include "../InstanceData.h"

namespace Renderer
{
    namespace Commands
    {
        struct DrawCommand : ICommand
        {
            DrawCommand(const ModelID _modelID, const InstanceData& _instanceData)
                : modelID(_modelID)
                , instanceData(_instanceData)
            {}

            const ModelID& modelID;
            const InstanceData& instanceData;
        };
    }
}