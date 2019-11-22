#include "ShaderHandler.h"
#include <cassert>
#include "d3dx12.h"
#include <fstream>
#include "RenderDeviceDX12.h"
#include <Utils/StringUtils.h>

ShaderHandler::~ShaderHandler()
{
    // TODO: Cleanup
    for (LoadedShader shader : _vertexShaders)
    {
        delete shader.shader;
    }
    for (LoadedShader shader : _pixelShaders)
    {
        delete shader.shader;
    }
    for (LoadedShader shader : _computeShaders)
    {
        delete shader.shader;
    }
}

__ShaderHandle ShaderHandler::LoadShader(const std::string& shaderPath, ShaderHandler::LoadedShaders& shaders)
{
    ID3DBlob* shaderBinary = ReadFile(shaderPath);
    size_t handle = shaders.size();

    // Make sure we haven't exceeded the limit of the ShaderHandle type, if this hits you need to change type of __ShaderHandle to something bigger
    assert(handle < __ShaderHandle::MaxValue());
    using type = type_safe::underlying_type<__ShaderHandle>;

    LoadedShader loadedShader;
    loadedShader.handle = __ShaderHandle(static_cast<type>(handle));
    loadedShader.shader = new CD3DX12_SHADER_BYTECODE(shaderBinary);
    loadedShader.name = shaderPath.substr(shaderPath.find_last_of("/\\") + 1);;
    loadedShader.path = shaderPath;

    shaders.push_back(loadedShader);

    return loadedShader.handle;
}

CD3DX12_SHADER_BYTECODE* ShaderHandler::GetShader(LoadedShaders& shaders, __ShaderHandle handle)
{
    using type = type_safe::underlying_type<__ShaderHandle>;

    // Lets make sure this handle exists
    assert(shaders.size() > static_cast<type>(handle));
    return shaders[static_cast<type>(handle)].shader;
}

ID3DBlob* ShaderHandler::ReadFile(const std::string& filename)
{
    std::wstring wideFilename = StringUtils::StringToWString(filename);
    ID3DBlob* blob;
    HRESULT result = D3DReadFileToBlob(wideFilename.c_str(), &blob);
    assert(SUCCEEDED(result));

    // TODO: This needs to get loaded to GPU somehow...
    /*std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    ShaderBinary buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();*/
    return blob;
}