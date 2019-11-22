#include "Model.h"
#include "DX12/d3dx12.h"

Model::Model()
    : _matrix()
    , _isBuffersCreated(false)
{
    // Create a quad for now
    _vertices.push_back(Vertex(Vector3(-0.5f, 0.5f, 0.5f), Vector4(1, 0, 0, 1))); // top left
    _vertices.push_back(Vertex(Vector3(0.5f, -0.5f, 0.5f), Vector4(0, 1, 0, 1))); // bottom right
    _vertices.push_back(Vertex(Vector3(-0.5f, -0.5f, 0.5f), Vector4(0, 0, 1, 1))); // bottom left
    _vertices.push_back(Vertex(Vector3(0.5f, 0.5f, 0.5f), Vector4(0, 1, 1, 1))); // top right

    // First triangle
    _indices.push_back(0);
    _indices.push_back(1);
    _indices.push_back(2);
    // Second triangle
    _indices.push_back(0);
    _indices.push_back(3);
    _indices.push_back(1);

    /*
        { -0.5f,  0.5f, 0.5f }, // top left
        {  0.5f, -0.5f, 0.5f }, // bottom right
        { -0.5f, -0.5f, 0.5f }, // bottom left
        {  0.5f,  0.5f, 0.5f }  // top right
    */
}

Model::~Model()
{
    delete _vertexBufferView;
    delete _indexBufferView;

    SAFE_RELEASE(_vertexBuffer);
    SAFE_RELEASE(_indexBuffer);

    for (int i = 0; i < frameBufferCount; ++i)
    {
        SAFE_RELEASE(_constantBufferUploadHeap[i]);
    };
}