#include "Model.h"
#include "DX12/d3dx12.h"
#include <Utils/StringUtils.h>

#pragma warning(push)
#pragma warning(disable: 4100 4244 4267 4127 4245 4521) // CapNProto doesn't give a fuck about warnings :(
#include <Types/CapModel.capnp.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#pragma warning(pop)


Model::Model()
    : _matrix()
    , _isBuffersCreated(false)
    , _constantBuffer()
{
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

bool Model::LoadFromFile(const std::string& filePath)
{
    // How to open files with handles for Capnproto, don't use fstream etc! https://www.mail-archive.com/capnproto@googlegroups.com/msg01052.html
    const std::wstring& wFilePath = StringUtils::StringToWString(filePath);
    HANDLE hFile = CreateFile(wFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    kj::HandleInputStream iStream(hFile);
    capnp::InputStreamMessageReader iStreamReader(iStream);

    CapModel::Reader capModel = iStreamReader.getRoot<CapModel>();
    
    // Read vertex positions
    for (CapVertex::Reader capVertex : capModel.getVertices())
    {
        Vertex vertex;
        vertex.pos = Vector3(capVertex.getPosition().getX(), capVertex.getPosition().getY(), capVertex.getPosition().getZ());
        _vertices.push_back(vertex);
    }

    // Read indices
    for (u32 index : capModel.getIndices())
    {
        _indices.push_back(index);
    }

    CloseHandle(hFile);
    return true;
}

void Model::MakeQuad()
{
    _vertices.push_back(Vertex(Vector3(-0.5f, 0.5f, 0.5f))); // top left
    _vertices.push_back(Vertex(Vector3(0.5f, -0.5f, 0.5f))); // bottom right
    _vertices.push_back(Vertex(Vector3(-0.5f, -0.5f, 0.5f))); // bottom left
    _vertices.push_back(Vertex(Vector3(0.5f, 0.5f, 0.5f))); // top right

    // First triangle
    _indices.push_back(0);
    _indices.push_back(1);
    _indices.push_back(2);
    // Second triangle
    _indices.push_back(0);
    _indices.push_back(3);
    _indices.push_back(1);
}