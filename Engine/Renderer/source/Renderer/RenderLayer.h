#pragma once
#include <Core.h>
#include <vector>
#include <Utils/RobinHood.h>
#include "InstanceData.h"
#include "Descriptors/ModelDesc.h"

namespace Renderer
{
    // A RenderLayer is just a collection of models to be drawn at certain positions
    class RenderLayer
    {
    public:
        using Instances = std::vector<InstanceData>;
        using _ModelID = type_safe::underlying_type<ModelID>;

        void RegisterModel(ModelID modelID, InstanceData& instanceData) { _models[static_cast<_ModelID>(modelID)].push_back(instanceData); }
        void Reset() { _models.clear(); }

        robin_hood::unordered_map<_ModelID, Instances>& GetModels() { return _models; }
    private:
        RenderLayer(){}

    private:
        robin_hood::unordered_map<_ModelID, Instances> _models;

        friend class Renderer;
    };

    /* This should be able to be drawn like this:
        
        RenderPass should:
            - Set RasterizerState
            * For each RenderLayer drawn in this pass
                * For each model in _models
                    - Set vertex/index buffers from ModelID
                    * For each InstanceData in model
                        - Set constant buffer
                        - DRAW
    
    */
}