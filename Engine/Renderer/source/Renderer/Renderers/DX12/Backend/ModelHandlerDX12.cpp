#include "ModelHandlerDX12.h"
#include "RenderDeviceDX12.h"
#include "d3dx12.h"
#include <Utils/StringUtils.h>
#include <cassert>
#include "CommandListHandlerDX12.h"

#pragma warning(push)
#pragma warning(disable: 4100 4244 4267 4127 4245 4521) // CapNProto doesn't give a fuck about warnings :(
#include <Types/CapModel.capnp.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#pragma warning(pop)

namespace Renderer
{
    namespace Backend
    {
        ModelHandlerDX12::ModelHandlerDX12()
        {

        }

        ModelHandlerDX12::~ModelHandlerDX12()
        {
            for (auto& model : _models)
            {
                SAFE_RELEASE(model.indexBuffer);
                SAFE_RELEASE(model.vertexBuffer);
            }
            _models.clear();
        }

        ModelID ModelHandlerDX12::LoadModel(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, const ModelDesc& desc)
        {
            size_t handle = _models.size();
            assert(handle < ModelID::MaxValue());
            using type = type_safe::underlying_type<ModelID>;

            Model model;
            model.desc = desc;

            TempModelData modelData;
            LoadFromFile(desc, modelData);
            InitModel(device, commandListHandler, model, modelData);

            _models.push_back(model);

            return ModelID(static_cast<type>(handle));
        }

        D3D12_VERTEX_BUFFER_VIEW* ModelHandlerDX12::GetVertexBufferView(ModelID id)
        {
            using type = type_safe::underlying_type<ModelID>;

            return _models[static_cast<type>(id)].vertexBufferView;
        }

        D3D12_INDEX_BUFFER_VIEW* ModelHandlerDX12::GetIndexBufferView(ModelID id)
        {
            using type = type_safe::underlying_type<ModelID>;

            return _models[static_cast<type>(id)].indexBufferView;
        }

        u32 ModelHandlerDX12::GetNumIndices(ModelID id)
        {
            using type = type_safe::underlying_type<ModelID>;

            return _models[static_cast<type>(id)].numIndices;
        }

        void ModelHandlerDX12::LoadFromFile(const ModelDesc& desc, TempModelData& data)
        {
            // How to open files with handles for Capnproto, don't use fstream etc! https://www.mail-archive.com/capnproto@googlegroups.com/msg01052.html
            const std::wstring& wFilePath = StringUtils::StringToWString(desc.path);
            HANDLE hFile = CreateFile(wFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

            kj::HandleInputStream iStream(hFile);
            capnp::InputStreamMessageReader iStreamReader(iStream);

            CapModel::Reader capModel = iStreamReader.getRoot<CapModel>();

            // Read vertex positions
            for (CapVertex::Reader capVertex : capModel.getVertices())
            {
                Vertex vertex(Vector3(capVertex.getPosition().getX(), capVertex.getPosition().getY(), capVertex.getPosition().getZ()));
                data.vertices.push_back(vertex);
            }

            // Read indices
            for (u32 index : capModel.getIndices())
            {
                data.indices.push_back(index);
            }

            CloseHandle(hFile);
        }

        void ModelHandlerDX12::InitModel(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, Model& model, TempModelData& data)
        {
            CommandListID commandListID = commandListHandler->BeginCommandList(device);
            ID3D12GraphicsCommandList* commandList = commandListHandler->GetCommandList(commandListID);

            HRESULT result;

            // -- VERTEX BUFFER --
            size_t vertexBufferSize = data.vertices.size() * sizeof(Vertex);
            {
                // Create vertex buffer
                result = device->_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
                    D3D12_RESOURCE_STATE_COPY_DEST, // We will start this heap in the copy destination state since we will copy data into this heap
                    nullptr,
                    IID_PPV_ARGS(&model.vertexBuffer)
                );
                assert(SUCCEEDED(result)); // Failed to create vertex buffer heap

                result = model.vertexBuffer->SetName(L"Vertex Buffer Resource Heap");
                assert(SUCCEEDED(result)); // Failed to name vertex buffer heap

                // Create upload buffer
                ID3D12Resource* vertexBufferUploadHeap;
                result = device->_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
                    D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
                    nullptr,
                    IID_PPV_ARGS(&vertexBufferUploadHeap));
                assert(SUCCEEDED(result)); // Faiuled to create vertex upload buffer heap

                result = vertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
                assert(SUCCEEDED(result)); // Failed to name vertex upload buffer heap

                // Store vertex buffer in upload buffer
                D3D12_SUBRESOURCE_DATA vertexData = {};
                vertexData.pData = reinterpret_cast<BYTE*>(data.vertices.data());
                vertexData.RowPitch = vertexBufferSize;
                vertexData.SlicePitch = vertexBufferSize;

                // Copy data from upload buffer to vertex buffer
                UpdateSubresources(commandList, model.vertexBuffer, vertexBufferUploadHeap, 0, 0, 1, &vertexData);

                // Transition the vertex buffer from copy destination state to vertex buffer state
                commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(model.vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
            }

            // -- INDEX BUFFER --
            size_t indexBufferSize = data.indices.size() * sizeof(u32);
            {
                // Create index buffer
                result = device->_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
                    D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data from the upload heap to this heap
                    nullptr,
                    IID_PPV_ARGS(&model.indexBuffer));
                assert(SUCCEEDED(result)); // Failed to create index buffer

                result = model.indexBuffer->SetName(L"Index Buffer Resource Heap");
                assert(SUCCEEDED(result)); // Failed to name index buffer

                // Create upload buffer
                ID3D12Resource* indexBufferUploadHeap;
                result = device->_device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
                    D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
                    nullptr,
                    IID_PPV_ARGS(&indexBufferUploadHeap));
                assert(SUCCEEDED(result)); // Failed to create index buffer upload heap

                result = indexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
                assert(SUCCEEDED(result)); // Failed to name index buffer upload heap

                // Store index buffer in upload buffer
                D3D12_SUBRESOURCE_DATA indexData = {};
                indexData.pData = reinterpret_cast<BYTE*>(data.indices.data()); // pointer to our index array
                indexData.RowPitch = indexBufferSize; // size of all our triangle index data
                indexData.SlicePitch = indexBufferSize; // also the size of our triangle index data

                // Copy data from upload buffer to index buffer
                UpdateSubresources(commandList, model.indexBuffer, indexBufferUploadHeap, 0, 0, 1, &indexData);

                // Transition the index buffer from copy destination state to index buffer state
                commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(model.indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
            }

            // Create a vertex buffer view
            model.vertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
            model.vertexBufferView->BufferLocation = model.vertexBuffer->GetGPUVirtualAddress();
            model.vertexBufferView->StrideInBytes = sizeof(Vertex);
            model.vertexBufferView->SizeInBytes = (UINT)vertexBufferSize;

            // Create a index buffer view
            model.indexBufferView = new D3D12_INDEX_BUFFER_VIEW();
            model.indexBufferView->BufferLocation = model.indexBuffer->GetGPUVirtualAddress();
            model.indexBufferView->Format = DXGI_FORMAT_R32_UINT;
            model.indexBufferView->SizeInBytes = (UINT)indexBufferSize;

            // Set number of indices
            model.numIndices = static_cast<u32>(data.indices.size());

            commandListHandler->EndCommandList(device, commandListID);
        }

    }
}