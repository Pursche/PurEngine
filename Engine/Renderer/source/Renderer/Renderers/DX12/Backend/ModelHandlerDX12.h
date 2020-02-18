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
        class CommandListHandlerDX12;

        class ModelHandlerDX12
        {
        public:
            ModelHandlerDX12();
            ~ModelHandlerDX12();

            ModelID LoadModel(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, const ModelDesc& desc);

            D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(ModelID id);
            D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView(ModelID id);
            u32 GetNumIndices(ModelID id);

        private:
            struct Model
            {
                ModelDesc desc;

                Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
                Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

                D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
                D3D12_INDEX_BUFFER_VIEW indexBufferView;

                u32 numIndices;
            };

            struct Vertex
            {
                Vertex(Vector3 inPos, Vector3 inNormal, Vector2 inTexCoord)
                {
                    pos = inPos;
                    normal = inNormal;
                    texCoord = inTexCoord;
                }

                Vector3 pos;
                Vector3 normal;
                Vector2 texCoord;
            };

            struct TempModelData
            {
                std::vector<Vertex> vertices;
                std::vector<u32> indices;
            };

        private:
            void LoadFromFile(const ModelDesc& desc, TempModelData& data);
            void InitModel(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, Model& model, TempModelData& data);
            
        private:
            std::vector<Model> _models;
        };
    }
}