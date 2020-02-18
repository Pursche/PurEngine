#pragma once
#include <Core.h>
#include <vector>
#include "d3dx12.h"

#include "../../../Descriptors/ImageDesc.h"
#include "../../../Descriptors/MaterialDesc.h"
#include "../../../Descriptors/GraphicsPipelineDesc.h"
#include "../../../Descriptors/VertexShaderDesc.h"
#include "../../../Descriptors/PixelShaderDesc.h"

#pragma warning(push)
#pragma warning(disable: 4100 4244 4267 4127 4245 4521) // CapNProto doesn't give a fuck about warnings :(
#include <Types/CapMaterial.capnp.h>
#pragma warning(pop)

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;
        class CommandListHandlerDX12;
        class ImageHandlerDX12;
        class ShaderHandlerDX12;
        class PipelineHandlerDX12;

        class MaterialHandlerDX12
        {
            using _MaterialID = type_safe::underlying_type<MaterialID>;
        public:
            MaterialHandlerDX12();
            ~MaterialHandlerDX12();

            MaterialID LoadMaterial(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, const MaterialDesc& desc);



            VertexShaderID GetVertexShader(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].vertexShader; }
            PixelShaderID GetPixelShader(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].pixelShader; }
            
            BlendState& GetBlendState(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].blendState; }

            u8 GetNumConstantBufferStates(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].numConstantBufferStates; }
            ConstantBufferState& GetConstantBufferState(const MaterialID id, int index) { return _materials[static_cast<_MaterialID>(id)].constantBufferStates[index]; }

            u8 GetNumInputLayouts(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].numInputLayouts; }
            InputLayout& GetInputLayout(const MaterialID id, int index) { return _materials[static_cast<_MaterialID>(id)].inputLayouts[index]; }

            u8 GetNumTextures(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].numTextures; }
            TextureID GetTexture(const MaterialID id, u32 index) { return _materials[static_cast<_MaterialID>(id)].textures[index]; }

            u8 GetNumSamplers(const MaterialID id) { return _materials[static_cast<_MaterialID>(id)].numSamplers; }
            Sampler GetSampler(const MaterialID id, u32 index) { return _materials[static_cast<_MaterialID>(id)].samplers[index]; }

        private:
            struct MaterialSampler
            {
                std::string name;
                CapFilterMode filterXY;
                CapFilterMode filterZ;
                CapWrapMode wrapModeXY;
                CapWrapMode wrapModeZ;
            };

            struct Blender
            {
                std::string name;
                bool enabled;
                bool logicOpEnabled;
                CapBlendMode srcBlendMode;
                CapBlendMode destBlendMode;
                CapBlendOp blendOp;
                CapBlendMode srcAlphaBlendMode;
                CapBlendMode destAlphaBlendMode;
                CapBlendOp alphaBlendOp;
            };

            struct Parameter
            {
                std::string name;
                CapParameterType type;
                CapSubType subType;
            };

            struct Input
            {
                std::string name;
                CapInputType type;
            };

            struct Output
            {
                std::string name;
                CapOutputType type;
                CapSubType subType;
                Blender blender;
            };

            struct MaterialHeader
            {
                std::string name;
                std::vector<Parameter> parameters;
                std::vector<MaterialSampler> samplers;
                std::vector<Input> inputs;
                std::vector<Output> outputs;
            };

            struct MaterialTexture
            {
                std::string name;
                std::string path;
                std::string contentPath;
            };
            
            struct MaterialInstanceHeader
            {
                std::string name;
                MaterialHeader materialHeader;
                std::vector<MaterialTexture> materialTextures;
            };

            struct Material
            {
                MaterialDesc desc;
                MaterialInstanceHeader header;

                VertexShaderID vertexShader;
                PixelShaderID pixelShader;

                BlendState blendState;
                ConstantBufferState constantBufferStates[MAX_CONSTANT_BUFFERS];
                u8 numConstantBufferStates = 0;

                InputLayout inputLayouts[MAX_INPUT_LAYOUTS];
                u8 numInputLayouts = 0;

                Sampler samplers[MAX_BOUND_TEXTURES];
                u8 numSamplers = 0;

                TextureID textures[MAX_BOUND_TEXTURES];
                u8 numTextures = 0;
            };

        private:
            void LoadHeaderFromFile(const MaterialDesc& desc, MaterialInstanceHeader& header);
            void LoadShaders(const MaterialDesc& desc, ShaderHandlerDX12* shaderHandler, Material& material);
            void InitMaterial(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, ImageHandlerDX12* imageHandler, Material& material);

            // Helpers
            BlendMode ToBlendMode(CapBlendMode blendMode);
            BlendOp ToBlendOp(CapBlendOp blendOp);
            SamplerFilter ToSamplerFilter(CapFilterMode xy, CapFilterMode z);
            TextureAddressMode ToTextureAddressMode(CapWrapMode wrapMode);
            
        private:
            std::vector<Material> _materials;
        };
    }
}