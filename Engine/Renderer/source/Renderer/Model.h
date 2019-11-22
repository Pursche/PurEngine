#pragma once
#include <Core.h>
#include <vector>
#include "RenderSettings.h"

#pragma warning(push)
#pragma warning(disable: 4100 4244 4267 4127 4245) // CapNProto doesn't give a fuck about warnings :(
#include <Types/CapModel.capnp.h>
#pragma warning(pop)

struct ID3D12Resource;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

class Model
{
public:
    struct ConstantBuffer
    {
        Vector4 colorMultiplier;
    };

    struct Vertex
    {
        Vertex(Vector3 inPos, Vector4 inColor)
        {
            pos = inPos;
            color = inColor;
        }
        Vector3 pos;
        Vector4 color;
    };

    Model();
    ~Model();

    Matrix& GetMatrix() { return _matrix; }
    std::vector<Vertex>& GetVertices() { return _vertices; }
    std::vector<u32>& GetIndices() { return _indices; }
    ConstantBuffer& GetConstants() { return _constantBuffer; }

    bool IsBuffersCreated() { return _isBuffersCreated; }
    void SetBuffersCreated() { _isBuffersCreated = true; }

    ID3D12Resource*& GetVertexBuffer() { return _vertexBuffer; }
    D3D12_VERTEX_BUFFER_VIEW*& GetVertexBufferView() { return _vertexBufferView; }
    u32 GetNumVertices() { return (u32)_vertices.size(); }

    ID3D12Resource*& GetIndexBuffer() { return _indexBuffer; }
    D3D12_INDEX_BUFFER_VIEW*& GetIndexBufferView() { return _indexBufferView; }
    u32 GetNumIndices() { return (u32)_indices.size(); }

    ID3D12Resource*& GetConstantBuffer(u32 frameIndex) { return _constantBufferUploadHeap[frameIndex]; }
    ConstantBuffer*& GetConstantBufferGPUAdress(u32 frameIndex) { return _constantBufferGPUAddress[frameIndex]; }
private:

private:
    Matrix _matrix;
    std::vector<Vertex> _vertices;
    std::vector<u32> _indices;
    bool _isBuffersCreated;

    // TODO: Abstract these
    ConstantBuffer _constantBuffer;
    ID3D12Resource* _vertexBuffer;
    ID3D12Resource* _indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW* _vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW* _indexBufferView;
    ID3D12Resource* _constantBufferUploadHeap[frameBufferCount];
    ConstantBuffer* _constantBufferGPUAddress[frameBufferCount];
};