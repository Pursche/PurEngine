#pragma once
#include <Core.h>
#include <vector>
#include <Containers/RobinHood.h>
#include <unordered_map>
#include "InstanceData.h"
#include "Descriptors/ModelDesc.h"
#include "Descriptors/MaterialDesc.h"

namespace Renderer
{
    // A RenderLayer is just a collection of models to be drawn at certain positions
    class RenderLayer
    {
    public:
        typedef std::vector<InstanceData*> Instances;
        typedef type_safe::underlying_type<ModelID> _ModelID;
        typedef type_safe::underlying_type<MaterialID> _MaterialID;

        typedef robin_hood::unordered_map<_ModelID, Instances> Models;
        typedef robin_hood::unordered_map<_MaterialID, Models> Materials;

        void RegisterModel(MaterialID materialID, ModelID modelID, InstanceData* instanceData) { _materials[static_cast<_MaterialID>(materialID)][static_cast<_ModelID>(modelID)].push_back(instanceData); }
        void Reset() 
        {
            for (auto& material : _materials)
            {
                for (auto& model : material.second)
                {
                    model.second.clear();
                }
                material.second.clear();
            }

            _materials.clear();
        }

        //robin_hood::unordered_map<_ModelID, Instances>& GetModels() { return _models; }
        //robin_hood::unordered_map<_ModelID, Instances>& GetModels() { return _models; }
        Materials& GetMaterials() { return _materials; }

        RenderLayer() {}
        ~RenderLayer()
        {
            Reset();
        }

    private:
        

    private:
        //robin_hood::unordered_map<_ModelID, Instances> _models;
        Materials _materials;
        //std::unordered_map<_ModelID, Instances> _models;
    };
}