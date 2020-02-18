using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml;

namespace Cooker.Cookers
{
    static class MaterialParser
    {
        public static bool ParseSampler(XmlNode samplerNode, List<CapnpGen.CapSampler> samplers, out string error)
        {
            error = "";
            CapnpGen.CapSampler sampler = new CapnpGen.CapSampler();

            string name = "";
            if (!TryGetAttribute(samplerNode, "name", out name))
            {
                error = "Sampler node does not have a name attribute";
                return false;
            }
            sampler.Name = name;

            CapnpGen.CapFilterMode filterXY = CapnpGen.CapFilterMode.point;
            if (!TryGetAttribute(samplerNode, "filterXY", out filterXY))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid filterXY attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapFilterMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.FilterXY = filterXY;

            CapnpGen.CapFilterMode filterZ = CapnpGen.CapFilterMode.point;
            if (!TryGetAttribute(samplerNode, "filterZ", out filterZ))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid filterZ attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapFilterMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.FilterZ = filterZ;

            CapnpGen.CapWrapMode wrapModeXY = CapnpGen.CapWrapMode.clamp;
            if (!TryGetAttribute(samplerNode, "wrapModeXY", out wrapModeXY))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid wrapModeXY attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapWrapMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.WrapModeXY = wrapModeXY;

            CapnpGen.CapWrapMode wrapModeZ = CapnpGen.CapWrapMode.clamp;
            if (!TryGetAttribute(samplerNode, "wrapModeZ", out wrapModeZ))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid wrapModeZ attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapWrapMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.WrapModeZ = wrapModeZ;

            samplers.Add(sampler);
            return true;
        }

        public static bool ParseBlender(XmlNode blenderNode, List<CapnpGen.CapBlender> blenders, out string error)
        {
            error = "";
            CapnpGen.CapBlender blender = new CapnpGen.CapBlender();

            string name = "";
            if (!TryGetAttribute(blenderNode, "name", out name))
            {
                error = "Blender node does not have a name attribute";
                return false;
            }
            blender.Name = name;

            bool enabled = false;
            if (!TryGetAttribute(blenderNode, "enabled", out enabled))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid enabled attribute, valid values: true false";
                return false;
            }
            blender.Enabled = enabled;

            bool logicOpEnabled = false;
            if (!TryGetAttribute(blenderNode, "logicOpEnabled", out logicOpEnabled))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid logicOpEnabled attribute, valid values: true false";
                return false;
            }
            blender.LogicOpEnabled = logicOpEnabled;

            CapnpGen.CapBlendMode srcBlendMode = CapnpGen.CapBlendMode.zero;
            if (!TryGetAttribute(blenderNode, "srcBlendMode", out srcBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid srcBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.SrcBlendMode = srcBlendMode;

            CapnpGen.CapBlendMode destBlendMode = CapnpGen.CapBlendMode.zero;
            if (!TryGetAttribute(blenderNode, "destBlendMode", out destBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid destBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.DestBlendMode = destBlendMode;

            CapnpGen.CapBlendOp blendOp = CapnpGen.CapBlendOp.add;
            if (!TryGetAttribute(blenderNode, "blendOp", out blendOp))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid blendOp attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendOp));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.BlendOp = blendOp;

            CapnpGen.CapBlendMode srcAlphaBlendMode = CapnpGen.CapBlendMode.zero;
            if (!TryGetAttribute(blenderNode, "srcAlphaBlendMode", out srcAlphaBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid srcAlphaBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.SrcAlphaBlendMode = srcAlphaBlendMode;

            CapnpGen.CapBlendMode destAlphaBlendMode = CapnpGen.CapBlendMode.zero;
            if (!TryGetAttribute(blenderNode, "destAlphaBlendMode", out destAlphaBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid destAlphaBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.DestAlphaBlendMode = destAlphaBlendMode;

            CapnpGen.CapBlendOp alphaBlendOp = CapnpGen.CapBlendOp.add;
            if (!TryGetAttribute(blenderNode, "alphaBlendOp", out alphaBlendOp))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid alphaBlendOp attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapBlendOp));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.AlphaBlendOp = alphaBlendOp;

            blenders.Add(blender);
            return true;
        }

        public static bool ParseMaterial(XmlNode materialNode, List<CapnpGen.CapMaterial> materials, List<CapnpGen.CapBlender> blenders, List<CapnpGen.CapSampler> samplers, out string error)
        {
            error = "";
            CapnpGen.CapMaterial material = new CapnpGen.CapMaterial();
            material.Header = new CapnpGen.CapMaterialHeader();
            material.Header.Samplers = samplers;

            // Name
            string name = "";
            if (!TryGetAttribute(materialNode, "name", out name))
            {
                error = "Material node does not have a name attribute";
                return false;
            }
            material.Header.Name = name;

            // Parameters
            List<CapnpGen.CapParameter> parameters = new List<CapnpGen.CapParameter>();
            XmlNodeList parameterNodes = materialNode.SelectNodes("Parameter");
            foreach (XmlNode parameterNode in parameterNodes)
            {
                if (!ParseParameter(parameterNode, parameters, out error))
                {
                    return false;
                }
            }
            material.Header.Parameters = parameters;

            // Inputs
            List<CapnpGen.CapInput> inputs = new List<CapnpGen.CapInput>();
            XmlNodeList inputNodes = materialNode.SelectNodes("Input");
            foreach (XmlNode inputNode in inputNodes)
            {
                if (!ParseInput(inputNode, inputs, out error))
                {
                    return false;
                }
            }
            material.Header.Inputs = inputs;

            // Outputs
            List<CapnpGen.CapOutput> outputs = new List<CapnpGen.CapOutput>();
            XmlNodeList outputNodes = materialNode.SelectNodes("Output");
            foreach (XmlNode outputNode in outputNodes)
            {
                if (!ParseOutput(outputNode, outputs, blenders, out error))
                {
                    return false;
                }
            }
            material.Header.Outputs = outputs;

            // PSBody
            XmlNode psBodyNode = materialNode.SelectSingleNode("PSBody");
            if (psBodyNode == null)
            {
                error = "Material node \"" + material.Header.Name + "\" does not have a PSBody node, this is required.";
                return false;
            }
            material.PsBody = psBodyNode.InnerText.Trim();

            materials.Add(material);
            return true;
        }

        public static bool ParseMaterialInstance(XmlNode materialInstanceNode, List<CapnpGen.CapMaterialInstance> materialInstances, List<CapnpGen.CapMaterial> materials, string materialPath, out string error)
        {
            error = "";
            CapnpGen.CapMaterialInstance materialInstance = new CapnpGen.CapMaterialInstance();
            materialInstance.Header = new CapnpGen.CapMaterialInstanceHeader();

            // Name
            string name = "";
            if (!TryGetAttribute(materialInstanceNode, "name", out name))
            {
                error = "MaterialInstance node does not have a name attribute";
                return false;
            }
            materialInstance.Header.Name = name;

            // Material
            string materialName = "";
            if (!TryGetAttribute(materialInstanceNode, "material", out materialName))
            {
                error = "MaterialInstance " + name + " node does not have a material attribute";
                return false;
            }

            // Find and verify the material
            bool found = false;
            foreach(CapnpGen.CapMaterial material in materials)
            {
                if (material.Header.Name == materialName)
                {
                    materialInstance.Material = material;
                    materialInstance.Header.MaterialHeader = material.Header;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                error = "MaterialInstance " + name + " node does not have a valid material attribute, provided material was " + materialName;
                return false;
            }

            // Textures
            List<CapnpGen.CapMaterialTexture> textures = new List<CapnpGen.CapMaterialTexture>();
            XmlNodeList textureNodes = materialInstanceNode.SelectNodes("Texture");
            foreach (XmlNode textureNode in textureNodes)
            {
                if (!ParseMaterialTexture(textureNode, textures, materialPath, out error))
                {
                    return false;
                }
            }
            materialInstance.Header.Textures = textures;

            // Verify that all provided Textures have a valid name for a material parameter
            foreach(CapnpGen.CapMaterialTexture texture in textures)
            {
                bool valid = false;
                foreach (CapnpGen.CapParameter parameter in materialInstance.Material.Header.Parameters)
                {
                    if (parameter.Name == texture.Name)
                    {
                        valid = true;
                        break;
                    }
                }

                if (!valid)
                {
                    error = "MaterialInstance " + name + " node has a texture" + texture.Name + " without a matching parameter in material " + materialInstance.Material.Header.Name;
                    return false;
                }
            }

            // Verify that all provided Textures has a valid path pointing to a file
            foreach (CapnpGen.CapMaterialTexture texture in textures)
            {
                if (!File.Exists(texture.ContentPath))
                {
                    error = "MaterialInstance " + name + " node has a texture" + texture.Name + " with the invalid path " + texture.Path;
                    return false;
                }
            }


            materialInstances.Add(materialInstance);
            return true;
        }

        static bool ParseMaterialTexture(XmlNode textureNode, List<CapnpGen.CapMaterialTexture> textures, string materialPath, out string error)
        {
            error = "";
            CapnpGen.CapMaterialTexture texture = new CapnpGen.CapMaterialTexture();

            // Name
            string name = "";
            if (!TryGetAttribute(textureNode, "name", out name))
            {
                error = "Texture node does not have a name attribute";
                return false;
            }
            texture.Name = name;

            // Path
            string path = "";
            if (!TryGetAttribute(textureNode, "path", out path))
            {
                error = "Texture node " + name + " does not have a path attribute";
                return false;
            }
            texture.Path = path;
            texture.ContentPath = Path.Combine(Path.GetDirectoryName(materialPath), path);

            textures.Add(texture);
            return true;
        }

        static bool ParseParameter(XmlNode parameterNode, List<CapnpGen.CapParameter> parameters, out string error)
        {
            error = "";
            CapnpGen.CapParameter parameter = new CapnpGen.CapParameter();

            string name = "";
            if (!TryGetAttribute(parameterNode, "name", out name))
            {
                error = "Parameter node does not have a name attribute";
                return false;
            }
            parameter.Name = name;

            CapnpGen.CapParameterType parameterType = CapnpGen.CapParameterType.@float;
            if (!TryGetAttribute(parameterNode, "type", out parameterType))
            {
                error = "Parameter node \"" + parameter.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapParameterType));
                foreach (var value in values)
                {
                    string valueString = value.ToString();
                    if (valueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                        valueString = valueString.Substring(1);

                    error += value + " ";
                }
                return false;
            }
            parameter.Type = parameterType;

            if (parameterType.ToString().StartsWith("texture"))
            {
                CapnpGen.CapSubType subType = CapnpGen.CapSubType.@float;
                if (!TryGetAttribute(parameterNode, "subtype", out subType))
                {
                    error = "Parameter node \"" + parameter.Name + "\" is of a texture type but does not have a valid subtype attribute, valid values: ";
                    var values = Enum.GetValues(typeof(CapnpGen.CapSubType));
                    foreach (var value in values)
                    {
                        string valueString = value.ToString();
                        if (valueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                            valueString = valueString.Substring(1);

                        error += value + " ";
                    }
                    return false;
                }
                parameter.SubType = subType;
            }

            parameters.Add(parameter);
            return true;
        }

        static bool ParseInput(XmlNode inputNode, List<CapnpGen.CapInput> inputs, out string error)
        {
            error = "";
            CapnpGen.CapInput input = new CapnpGen.CapInput();

            string name = "";
            if (!TryGetAttribute(inputNode, "name", out name))
            {
                error = "Input node does not have a name attribute";
                return false;
            }
            input.Name = name;

            CapnpGen.CapInputType inputType = CapnpGen.CapInputType.position;
            if (!TryGetAttribute(inputNode, "type", out inputType))
            {
                error = "Input node \"" + input.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapInputType));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            input.Type = inputType;

            inputs.Add(input);
            return true;
        }

        static bool ParseOutput(XmlNode outputNode, List<CapnpGen.CapOutput> outputs, List<CapnpGen.CapBlender> blenders, out string error)
        {
            error = "";
            CapnpGen.CapOutput output = new CapnpGen.CapOutput();

            string name = "";
            if (!TryGetAttribute(outputNode, "name", out name))
            {
                error = "Output node does not have a name attribute";
                return false;
            }
            output.Name = name;

            CapnpGen.CapOutputType outputType = CapnpGen.CapOutputType.color;
            if (!TryGetAttribute(outputNode, "type", out outputType))
            {
                error = "Output node \"" + output.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.CapOutputType));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            output.Type = outputType;

            output.Blender = null;
            // If output type was color we'll need a blender
            if (output.Type == CapnpGen.CapOutputType.color)
            {
                // Get the blender name
                string blenderName = "";
                if (!TryGetAttribute(outputNode, "blender", out blenderName))
                {
                    error = "Output node \"" + output.Name + "\" is of type Color but does not have a blender attribute. All color outputs need a blender attribute";
                    return false;
                }

                // Find and reference the blender itself
                foreach (CapnpGen.CapBlender blender in blenders)
                {
                    if (blender.Name == blenderName)
                    {
                        output.Blender = blender;
                        break;
                    }
                }

                // If we didn't find one, let's report an error
                if (output.Blender == null)
                {
                    error = "Output node \"" + output.Name + "\" references unknown blender \"" + blenderName + "\"";
                    return false;
                }

                CapnpGen.CapSubType subType = CapnpGen.CapSubType.@float;
                if (!TryGetAttribute(outputNode, "subtype", out subType))
                {
                    error = "Output node \"" + output.Name + "\" is of type Color but does not have a valid subtype attribute, valid values: ";
                    var values = Enum.GetValues(typeof(CapnpGen.CapSubType));
                    foreach (var value in values)
                    {
                        string valueString = value.ToString();
                        if (valueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                            valueString = valueString.Substring(1);

                        error += value + " ";
                    }
                    return false;
                }
                output.SubType = subType;
            }

            outputs.Add(output);
            return true;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out bool value)
        {
            value = false;

            if (node.Attributes[attribute] == null)
                return false;

            return bool.TryParse(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out string value)
        {
            value = "";

            if (node.Attributes[attribute] == null)
                return false;

            value = node.Attributes[attribute].Value;
            return true;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapFilterMode value)
        {
            value = CapnpGen.CapFilterMode.point;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapFilterMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapWrapMode value)
        {
            value = CapnpGen.CapWrapMode.clamp;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapWrapMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapBlendMode value)
        {
            value = CapnpGen.CapBlendMode.zero;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapBlendMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapBlendOp value)
        {
            value = CapnpGen.CapBlendOp.add;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapBlendOp>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapParameterType value)
        {
            value = CapnpGen.CapParameterType.@float;

            if (node.Attributes[attribute] == null)
                return false;

            string valueString = node.Attributes[attribute].Value;
            var enumValues = Enum.GetValues(typeof(CapnpGen.CapParameterType));
            foreach (var enumValue in enumValues)
            {
                string enumValueString = enumValue.ToString();
                if (enumValueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                {
                    enumValueString = enumValueString.Substring(1);
                }

                if (valueString == enumValueString)
                {
                    value = (CapnpGen.CapParameterType)enumValue;
                    return true;
                }
            }

            return false;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapSubType value)
        {
            value = CapnpGen.CapSubType.@float;

            if (node.Attributes[attribute] == null)
                return false;

            string valueString = node.Attributes[attribute].Value;
            var enumValues = Enum.GetValues(typeof(CapnpGen.CapSubType));
            foreach (var enumValue in enumValues)
            {
                string enumValueString = enumValue.ToString();
                if (enumValueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                {
                    enumValueString = enumValueString.Substring(1);
                }

                if (valueString == enumValueString)
                {
                    value = (CapnpGen.CapSubType)enumValue;
                    return true;
                }
            }

            return false;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapInputType value)
        {
            value = CapnpGen.CapInputType.position;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapInputType>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.CapOutputType value)
        {
            value = CapnpGen.CapOutputType.color;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.CapOutputType>(node.Attributes[attribute].Value, out value);
        }

    }
}