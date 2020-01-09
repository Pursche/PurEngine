#pragma once
#include <Core.h>
#include <vector>
#include "d3dx12.h"

#include "../../../Descriptors/ModelDesc.h"

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;

        class ModelHandlerDX12
        {
        public:
            ModelHandlerDX12();
            ~ModelHandlerDX12();

            ModelID LoadModel(RenderDeviceDX12* device, const ModelDesc& desc);

        private:
            struct Model
            {
                ModelDesc desc;

                ID3D12Resource* vertexBuffer;
                ID3D12Resource* indexBuffer;

                D3D12_VERTEX_BUFFER_VIEW* vertexBufferView;
                D3D12_INDEX_BUFFER_VIEW* indexBufferView;
            };

            struct Vertex
            {
                Vertex(Vector3 inPos)
                {
                    pos = inPos;
                }

                Vector3 pos;
            };

            struct TempModelData
            {
                std::vector<Vertex> vertices;
                std::vector<u32> indices;
            };

        private:
            void LoadFromFile(const ModelDesc& desc, TempModelData& data);
            void InitModel(RenderDeviceDX12* device, Model& model, TempModelData& data);
            
        private:
            std::vector<Model> _models;
        };
    }
}