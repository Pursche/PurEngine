#include "ShaderHandlerDX12.h"
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <Utils/StringUtils.h>

namespace Renderer
{
    namespace Backend
    {
        ShaderHandlerDX12::ShaderHandlerDX12()
        {

        }

        ShaderHandlerDX12::~ShaderHandlerDX12()
        {
            for (Shader& shader : _vertexShaders)
            {
                delete shader.bytecode;
            }
            for (Shader& shader : _pixelShaders)
            {
                delete shader.bytecode;
            }
            for (Shader& shader : _computeShaders)
            {
                delete shader.bytecode;
            }

            // TODO: Clean up all held resources
        }

        VertexShaderID ShaderHandlerDX12::LoadShader(const VertexShaderDesc& desc)
        {
            return LoadShader<VertexShaderID>(desc.path, _vertexShaders);
        }

        PixelShaderID ShaderHandlerDX12::LoadShader(const PixelShaderDesc& desc)
        {
            return LoadShader<PixelShaderID>(desc.path, _pixelShaders);
        }

        ComputeShaderID ShaderHandlerDX12::LoadShader(const ComputeShaderDesc& desc)
        {
            return LoadShader<ComputeShaderID>(desc.path, _computeShaders);
        }

        ID3DBlob* ShaderHandlerDX12::ReadFile(const std::string& filename)
        {
            std::wstring wideFilename = StringUtils::StringToWString(filename);
            ID3DBlob* blob;
            HRESULT result = D3DReadFileToBlob(wideFilename.c_str(), &blob);
            assert(SUCCEEDED(result));
            return blob;
        }

        bool ShaderHandlerDX12::TryFindExistingShader(const std::string& shaderPath, std::vector<Shader>& shaders, size_t& id)
        {
            u32 shaderPathHash = StringUtils::fnv1a_32(shaderPath.c_str(), shaderPath.length());

            id = 0;
            for (Shader& existingShader : shaders)
            {
                if (StringUtils::fnv1a_32(existingShader.path.c_str(), existingShader.path.length()) == shaderPathHash)
                {
                    return true;
                }
                id++;
            }

            return false;
        }


    }
}