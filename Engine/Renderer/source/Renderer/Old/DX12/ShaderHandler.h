#pragma once
#include <Core.h>
#include <vector>
#include <d3dcompiler.h>
#include "../ShaderHandle.h"

class RenderDeviceDX12;
struct ID3D12PipelineState;
struct CD3DX12_SHADER_BYTECODE;

class ShaderHandler
{
public:
    ShaderHandler() { };
    ~ShaderHandler();

    ShaderHandleVertex LoadShaderVertex(const std::string& shaderPath) { return static_cast<ShaderHandleVertex>(LoadShader(shaderPath, _vertexShaders)); }
    ShaderHandlePixel LoadShaderPixel(const std::string& shaderPath) { return static_cast<ShaderHandlePixel>(LoadShader(shaderPath, _pixelShaders)); }
    ShaderHandleCompute LoadShaderCompute(const std::string& shaderPath) { return static_cast<ShaderHandleCompute>(LoadShader(shaderPath, _computeShaders)); }

    CD3DX12_SHADER_BYTECODE* GetShaderVertex(ShaderHandleVertex handle) { return GetShader(_vertexShaders, static_cast<__ShaderHandle>(handle)); }
    CD3DX12_SHADER_BYTECODE* GetShaderPixel(ShaderHandlePixel handle) { return GetShader(_pixelShaders, static_cast<__ShaderHandle>(handle)); }
    CD3DX12_SHADER_BYTECODE* GetShaderCompute(ShaderHandleCompute handle) { return GetShader(_computeShaders, static_cast<__ShaderHandle>(handle)); }

private:
    struct LoadedShader
    {
        __ShaderHandle handle;
        CD3DX12_SHADER_BYTECODE* shader;
        std::string name;
        std::string path;
    };
    typedef std::vector<LoadedShader> LoadedShaders;

    __ShaderHandle LoadShader(const std::string& shaderPath, LoadedShaders& shaders);
    CD3DX12_SHADER_BYTECODE* GetShader(LoadedShaders& shaders, __ShaderHandle handle);
    
    ID3DBlob* ReadFile(const std::string& filename);
private:
    LoadedShaders _vertexShaders;
    LoadedShaders _pixelShaders;
    LoadedShaders _computeShaders;
};