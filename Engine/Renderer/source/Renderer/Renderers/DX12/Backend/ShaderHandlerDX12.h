#pragma once
#include <Core.h>
#include <vector>
#include <cassert>
#include "d3dx12.h"

#include "../../../Descriptors/VertexShaderDesc.h"
#include "../../../Descriptors/PixelShaderDesc.h"
#include "../../../Descriptors/ComputeShaderDesc.h"

namespace Renderer
{
    namespace Backend
    {        
        class ShaderHandlerDX12
        {
            using vsIDType = type_safe::underlying_type<VertexShaderID>;
            using psIDType = type_safe::underlying_type<PixelShaderID>;
            using csIDType = type_safe::underlying_type<ComputeShaderID>;

        public:
            ShaderHandlerDX12();
            ~ShaderHandlerDX12();

            VertexShaderID LoadShader(const VertexShaderDesc& desc);
            PixelShaderID LoadShader(const PixelShaderDesc& desc);
            ComputeShaderID LoadShader(const ComputeShaderDesc& desc);

            CD3DX12_SHADER_BYTECODE* GetBytecode(const VertexShaderID id) { return _vertexShaders[static_cast<vsIDType>(id)].bytecode; }
            CD3DX12_SHADER_BYTECODE* GetBytecode(const PixelShaderID id) { return _pixelShaders[static_cast<psIDType>(id)].bytecode; }
            CD3DX12_SHADER_BYTECODE* GetBytecode(const ComputeShaderID id) { return _computeShaders[static_cast<csIDType>(id)].bytecode; }

        private:
            struct Shader
            {
                CD3DX12_SHADER_BYTECODE* bytecode;
                std::string path;
            };

        private:
            template <typename T>
            T LoadShader(const std::string& shaderPath, std::vector<Shader>& shaders)
            {
                size_t id;
                using idType = type_safe::underlying_type<T>;

                // If shader is already loaded, return ID of already loaded version
                if (TryFindExistingShader(shaderPath, shaders, id))
                {
                    return T(static_cast<idType>(id));
                }

                ID3DBlob* bytecode = ReadFile(shaderPath);
                id = shaders.size();

                assert(id < T::MaxValue());
                
                Shader shader;
                shader.bytecode = new CD3DX12_SHADER_BYTECODE(bytecode);
                shader.path = shaderPath;

                shaders.push_back(shader);

                return T(static_cast<idType>(id));
            }

            ID3DBlob* ReadFile(const std::string& filename);
            bool TryFindExistingShader(const std::string& shaderPath, std::vector<Shader>& shaders, size_t& id);
            
        private:
            std::vector<Shader> _vertexShaders;
            std::vector<Shader> _pixelShaders;
            std::vector<Shader> _computeShaders;
        };
    }
}