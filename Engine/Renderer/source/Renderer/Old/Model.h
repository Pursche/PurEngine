#pragma once
#include <Core.h>
#include <vector>
#include "RenderSettings.h"

struct ID3D12Resource;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

class Model
{
public:
    // This struct needs to be 256 bytes padded
    struct ConstantBuffer
    {
        Vector4 colorMultiplier; // 16 bytes
        Matrix modelMatrix; // 64 bytes
        float padding[176];
    };

    struct Vertex
    {
        Vertex()
        {}
        Vertex(Vector3 inPos)
        {
            pos = inPos;
        }
        Vector3 pos;
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

    bool LoadFromFile(const std::string& filePath);
    void MakeQuad();
private:
    
    
private:
    Matrix _matrix;
    std::vector<Vertex> _vertices;
    std::vector<u32> _indices;
    bool _isBuffersCreated;
    ConstantBuffer _constantBuffer;

    // TODO: Abstract these
    ID3D12Resource* _vertexBuffer;
    ID3D12Resource* _indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW* _vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW* _indexBufferView;
    ID3D12Resource* _constantBufferUploadHeap[frameBufferCount];
    ConstantBuffer* _constantBufferGPUAddress[frameBufferCount];
};