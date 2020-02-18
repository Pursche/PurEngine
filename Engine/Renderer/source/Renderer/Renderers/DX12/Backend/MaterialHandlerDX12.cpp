#include "MaterialHandlerDX12.h"
#include "RenderDeviceDX12.h"
#include "ImageHandlerDX12.h"
#include "ShaderHandlerDX12.h"
#include "d3dx12.h"
#include <Utils/StringUtils.h>
#include <cassert>

#pragma warning(push)
#pragma warning(disable: 4100 4244 4267 4127 4245 4521) // CapNProto doesn't give a fuck about warnings :(
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#pragma warning(pop)

namespace Renderer
{
    namespace Backend
    {
        MaterialHandlerDX12::MaterialHandlerDX12()
        {

        }

        MaterialHandlerDX12::~MaterialHandlerDX12()
        {
            
        }

        MaterialID MaterialHandlerDX12::LoadMaterial(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, ShaderHandlerDX12* shaderHandler, ImageHandlerDX12* imageHandler, const MaterialDesc& desc)
        {
            size_t handle = _materials.size();
            assert(handle < MaterialID::MaxValue());
            using type = type_safe::underlying_type<MaterialID>;

            Material material;
            material.desc = desc;

            LoadHeaderFromFile(desc, material.header);
            LoadShaders(desc, shaderHandler, material);
            InitMaterial(device, commandListHandler, imageHandler, material);

            _materials.push_back(material);

            return MaterialID(static_cast<type>(handle));
        }

        void MaterialHandlerDX12::LoadHeaderFromFile(const MaterialDesc& desc, MaterialInstanceHeader& header)
        {
            // How to open files with handles for Capnproto, don't use fstream etc! https://www.mail-archive.com/capnproto@googlegroups.com/msg01052.html
            const std::wstring& wFilePath = StringUtils::StringToWString(desc.path);
            HANDLE hFile = CreateFile(wFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

            kj::HandleInputStream iStream(hFile);
            capnp::InputStreamMessageReader iStreamReader(iStream);

            CapMaterialInstanceHeader::Reader headerReader = iStreamReader.getRoot<CapMaterialInstanceHeader>();
            
            // Read name
            header.name = headerReader.getName().cStr();

            // -- Read Material header --
            CapMaterialHeader::Reader materialHeaderReader = headerReader.getMaterialHeader();

            // Read Material name
            header.materialHeader.name = materialHeaderReader.getName().cStr();

            // Read parameters
            for (CapParameter::Reader parameterReader : materialHeaderReader.getParameters())
            {
                Parameter parameter;
                parameter.name = parameterReader.getName().cStr();
                parameter.type = parameterReader.getType();
                parameter.subType = parameterReader.getSubType();

                header.materialHeader.parameters.push_back(parameter);
            }

            // Read samplers
            for (CapSampler::Reader samplerReader : materialHeaderReader.getSamplers())
            {
                MaterialSampler sampler;
                sampler.name = samplerReader.getName().cStr();
                sampler.filterXY = samplerReader.getFilterXY();
                sampler.filterZ = samplerReader.getFilterZ();
                sampler.wrapModeXY = samplerReader.getWrapModeXY();
                sampler.wrapModeZ = samplerReader.getWrapModeZ();

                header.materialHeader.samplers.push_back(sampler);
            }

            // Read inputs
            for (CapInput::Reader inputReader : materialHeaderReader.getInputs())
            {
                Input input;
                input.name = inputReader.getName().cStr();
                input.type = inputReader.getType();

                header.materialHeader.inputs.push_back(input);
            }

            // Read outputs
            for (CapOutput::Reader outputReader : materialHeaderReader.getOutputs())
            {
                Output output;
                output.name = outputReader.getName().cStr();
                output.type = outputReader.getType();
                output.subType = outputReader.getSubType();

                // Read blender
                CapBlender::Reader blenderReader = outputReader.getBlender();
                output.blender.name = blenderReader.getName().cStr();
                output.blender.enabled = blenderReader.getEnabled();
                output.blender.logicOpEnabled = blenderReader.getLogicOpEnabled();
                output.blender.srcBlendMode = blenderReader.getSrcBlendMode();
                output.blender.destBlendMode = blenderReader.getDestBlendMode();
                output.blender.blendOp = blenderReader.getBlendOp();
                output.blender.srcAlphaBlendMode = blenderReader.getSrcAlphaBlendMode();
                output.blender.destAlphaBlendMode = blenderReader.getDestAlphaBlendMode();
                output.blender.alphaBlendOp = blenderReader.getAlphaBlendOp();

                header.materialHeader.outputs.push_back(output);
            }

            // -- Read Material textures --
            for (CapMaterialTexture::Reader textureReader : headerReader.getTextures())
            {
                MaterialTexture texture;
                texture.name = textureReader.getName();
                texture.path = textureReader.getPath();
                texture.contentPath = textureReader.getContentPath();

                header.materialTextures.push_back(texture);
            }

            CloseHandle(hFile);
        }

        void MaterialHandlerDX12::LoadShaders(const MaterialDesc& desc, ShaderHandlerDX12* shaderHandler, Material& material)
        {
            // desc.path looks like this: example/path/exampleMaterial.material
            std::string directory;
            const size_t last_slash_idx = desc.path.rfind('/');
            if (std::string::npos != last_slash_idx)
            {
                directory = desc.path.substr(0, last_slash_idx+1);
            }
            // Now directory looks like this: example/path/

            std::string materialPath = directory + material.header.materialHeader.name;
            // materialPath now looks like this: example/path/exampleMaterial

            // Load vertex shader
            VertexShaderDesc vsDesc;
            vsDesc.path = materialPath + ".vs.hlsl.cso";
            material.vertexShader = shaderHandler->LoadShader(vsDesc);

            // Load pixel shader
            PixelShaderDesc psDesc;
            psDesc.path = materialPath + ".ps.hlsl.cso";
            material.pixelShader = shaderHandler->LoadShader(psDesc);
        }

        void MaterialHandlerDX12::InitMaterial(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, ImageHandlerDX12* imageHandler, Material& material)
        {
            // Blend states
            material.blendState.independentBlendEnable = true;
            for (int i = 0; i < material.header.materialHeader.outputs.size(); i++)
            {
                Blender& blender = material.header.materialHeader.outputs[i].blender;

                material.blendState.renderTargets[i].blendEnable = blender.enabled;
                material.blendState.renderTargets[i].logicOpEnable = blender.logicOpEnabled;

                material.blendState.renderTargets[i].srcBlend = ToBlendMode(blender.srcBlendMode);
                material.blendState.renderTargets[i].destBlend = ToBlendMode(blender.srcBlendMode);
                material.blendState.renderTargets[i].blendOp = ToBlendOp(blender.blendOp);

                material.blendState.renderTargets[i].srcBlendAlpha = ToBlendMode(blender.srcAlphaBlendMode);
                material.blendState.renderTargets[i].destBlendAlpha = ToBlendMode(blender.destAlphaBlendMode);
                material.blendState.renderTargets[i].blendOpAlpha = ToBlendOp(blender.alphaBlendOp);
                
                material.blendState.renderTargets[i].logicOp = LOGIC_OP_NOOP;
                material.blendState.renderTargets[i].renderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
            }

            // CB States
            // Materials have 3 constant buffers so far, ViewCB, ModelCB and MaterialCB
            material.constantBufferStates[0].enabled = true; // ViewCB
            material.constantBufferStates[0].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_VERTEX;
            material.constantBufferStates[1].enabled = true; // ModelCB
            material.constantBufferStates[1].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_VERTEX;
            material.constantBufferStates[2].enabled = true; // MaterialCB
            material.constantBufferStates[2].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_PIXEL;
            material.numConstantBufferStates = 3;

            // TODO: Optimize input layout depending on what the material needs?
            // Input layouts
            material.inputLayouts[0].enabled = true;
            material.inputLayouts[0].SetName("POSITION");
            material.inputLayouts[0].format = Renderer::InputFormat::INPUT_FORMAT_R32G32B32_FLOAT;
            material.inputLayouts[0].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;
            material.inputLayouts[1].enabled = true;
            material.inputLayouts[1].SetName("NORMAL");
            material.inputLayouts[1].format = Renderer::InputFormat::INPUT_FORMAT_R32G32B32_FLOAT;
            material.inputLayouts[1].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;
            material.inputLayouts[2].enabled = true;
            material.inputLayouts[2].SetName("TEXCOORD");
            material.inputLayouts[2].format = Renderer::InputFormat::INPUT_FORMAT_R32G32_FLOAT;
            material.inputLayouts[2].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;
            material.numInputLayouts = 3;

            // Samplers
            for (MaterialSampler& materialSampler : material.header.materialHeader.samplers)
            {
                u8 index = material.numSamplers++;
                material.samplers[index].enabled = true;
                material.samplers[index].filter = ToSamplerFilter(materialSampler.filterXY, materialSampler.filterZ);
                material.samplers[index].addressU = ToTextureAddressMode(materialSampler.wrapModeXY);
                material.samplers[index].addressV = ToTextureAddressMode(materialSampler.wrapModeXY);
                material.samplers[index].addressW = ToTextureAddressMode(materialSampler.wrapModeZ);

                material.samplers[index].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_PIXEL;
            }

            // Load textures
            for (MaterialTexture& texture : material.header.materialTextures)
            {
                TextureDesc textureDesc;
                textureDesc.path = "Data/materials/" + texture.path;

                material.textures[material.numTextures++] = imageHandler->LoadTexture(device, commandListHandler, textureDesc);
            }
        }

        BlendMode MaterialHandlerDX12::ToBlendMode(CapBlendMode blendMode)
        {
            switch (blendMode)
            {
                case CapBlendMode::ZERO:                return BlendMode::BLEND_MODE_ZERO;
                case CapBlendMode::ONE:                 return BlendMode::BLEND_MODE_ONE;
                case CapBlendMode::SRC_COLOR:           return BlendMode::BLEND_MODE_SRC_COLOR;
                case CapBlendMode::INV_SRC_COLOR:       return BlendMode::BLEND_MODE_INV_SRC_COLOR;
                case CapBlendMode::SRC_ALPHA:           return BlendMode::BLEND_MODE_SRC_ALPHA;
                case CapBlendMode::INV_SRC_ALPHA:       return BlendMode::BLEND_MODE_INV_SRC_ALPHA;
                case CapBlendMode::DEST_COLOR:          return BlendMode::BLEND_MODE_DEST_COLOR;
                case CapBlendMode::INV_DEST_COLOR:      return BlendMode::BLEND_MODE_INV_DEST_COLOR;
                case CapBlendMode::DEST_ALPHA:          return BlendMode::BLEND_MODE_DEST_ALPHA;
                case CapBlendMode::INV_DEST_ALPHA:      return BlendMode::BLEND_MODE_INV_DEST_ALPHA;
                case CapBlendMode::BLEND_FACTOR:        return BlendMode::BLEND_MODE_BLEND_FACTOR;
                case CapBlendMode::INV_BLEND_FACTOR:    return BlendMode::BLEND_MODE_INV_BLEND_FACTOR;
                
                default:
                    assert(false); // Did we add a value to CapBlendMode without modifying this function?
            }

            return BlendMode::BLEND_MODE_ZERO;
        }

        BlendOp MaterialHandlerDX12::ToBlendOp(CapBlendOp blendOp)
        {
            switch (blendOp)
            {
                case CapBlendOp::ADD:           return BlendOp::BLEND_OP_ADD;
                case CapBlendOp::SUBTRACT:      return BlendOp::BLEND_OP_SUBTRACT;
                case CapBlendOp::REV_SUBTRACT:  return BlendOp::BLEND_OP_REV_SUBTRACT;
                case CapBlendOp::MIN:           return BlendOp::BLEND_OP_MIN;
                case CapBlendOp::MAX:           return BlendOp::BLEND_OP_MAX;

                default:
                    assert(false); // Did we add a value to CapBlendOp without modifying this function?
            }

            return BlendOp::BLEND_OP_ADD;
        }

        SamplerFilter MaterialHandlerDX12::ToSamplerFilter(CapFilterMode xy, CapFilterMode z)
        {
            if (xy == CapFilterMode::POINT && z == CapFilterMode::POINT)
            {
                return SAMPLER_FILTER_MIN_MAG_MIP_POINT;
            }
            else if (xy == CapFilterMode::POINT && z == CapFilterMode::LINEAR)
            {
                return SAMPLER_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (xy == CapFilterMode::POINT && z == CapFilterMode::ANISOTROPIC)
            {
                assert(false); // Unsupported filter mode, is this ever requested?
                return SAMPLER_FILTER_ANISOTROPIC;
            }
            else if (xy == CapFilterMode::LINEAR && z == CapFilterMode::POINT)
            {
                return SAMPLER_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (xy == CapFilterMode::LINEAR && z == CapFilterMode::LINEAR)
            {
                return SAMPLER_FILTER_MIN_MAG_MIP_LINEAR;
            }
            else if (xy == CapFilterMode::LINEAR && z == CapFilterMode::ANISOTROPIC)
            {
                assert(false); // Unsupported filter mode, is this ever requested?
                return SAMPLER_FILTER_ANISOTROPIC;
            }
            else if (xy == CapFilterMode::ANISOTROPIC && z == CapFilterMode::POINT)
            {
                assert(false); // Unsupported filter mode, is this ever requested?
                return SAMPLER_FILTER_ANISOTROPIC;
            }
            else if (xy == CapFilterMode::ANISOTROPIC && z == CapFilterMode::LINEAR)
            {
                assert(false); // Unsupported filter mode, is this ever requested?
                return SAMPLER_FILTER_ANISOTROPIC;
            }
            else if (xy == CapFilterMode::ANISOTROPIC && z == CapFilterMode::ANISOTROPIC)
            {
                return SAMPLER_FILTER_ANISOTROPIC;
            }

            assert(false); // Did we update CapFilterMode without changing this function?
            return SAMPLER_FILTER_MIN_MAG_MIP_POINT;
        }

        TextureAddressMode MaterialHandlerDX12::ToTextureAddressMode(CapWrapMode wrapMode)
        {
            switch(wrapMode)
            {
                case CapWrapMode::CLAMP:    return TextureAddressMode::TEXTURE_ADDRESS_MODE_CLAMP;
                case CapWrapMode::WRAP:     return TextureAddressMode::TEXTURE_ADDRESS_MODE_WRAP;
                case CapWrapMode::MIRROR:   return TextureAddressMode::TEXTURE_ADDRESS_MODE_MIRROR;
                case CapWrapMode::BORDER:   return TextureAddressMode::TEXTURE_ADDRESS_MODE_BORDER;
                default:
                    assert(false); // Did we update CapWrapMode without changing this function?
            }

            return TEXTURE_ADDRESS_MODE_CLAMP;
        }
    }
}