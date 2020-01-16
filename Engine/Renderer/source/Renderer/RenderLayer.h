#pragma once
#include <Core.h>
#include <vector>
#include <Containers/RobinHood.h>
#include <unordered_map>
#include "InstanceData.h"
#include "Descriptors/ModelDesc.h"

namespace Renderer
{
    // A RenderLayer is just a collection of models to be drawn at certain positions
    class RenderLayer
    {
    public:
        typedef std::vector<InstanceData*> Instances;
        typedef type_safe::underlying_type<ModelID> _ModelID;

        void RegisterModel(ModelID modelID, InstanceData* instanceData) { _models[static_cast<_ModelID>(modelID)].push_back(instanceData); }
        void Reset() 
        {
            for (auto& model : _models)
            {
                auto& instances = model.second;
                instances.clear();
            }

            _models.clear(); 
        }

        //robin_hood::unordered_map<_ModelID, Instances>& GetModels() { return _models; }
        std::unordered_map<_ModelID, Instances>& GetModels() { return _models; }

        RenderLayer() {}

        // Range based loop support

        
    private:
        

    private:
        //robin_hood::unordered_map<_ModelID, Instances> _models;
        std::unordered_map<_ModelID, Instances> _models;
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