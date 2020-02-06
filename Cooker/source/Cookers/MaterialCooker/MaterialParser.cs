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
        public static bool ParseSampler(XmlNode samplerNode, List<CapnpGen.Sampler> samplers, out string error)
        {
            error = "";
            CapnpGen.Sampler sampler = new CapnpGen.Sampler();

            string name = "";
            if (!TryGetAttribute(samplerNode, "name", out name))
            {
                error = "Sampler node does not have a name attribute";
                return false;
            }
            sampler.Name = name;

            CapnpGen.FilterMode filterXY = CapnpGen.FilterMode.point;
            if (!TryGetAttribute(samplerNode, "filterXY", out filterXY))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid filterXY attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.FilterMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.FilterXY = filterXY;

            CapnpGen.FilterMode filterZ = CapnpGen.FilterMode.point;
            if (!TryGetAttribute(samplerNode, "filterZ", out filterZ))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid filterZ attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.FilterMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.FilterZ = filterZ;

            CapnpGen.WrapMode wrapModeXY = CapnpGen.WrapMode.clamp;
            if (!TryGetAttribute(samplerNode, "wrapModeXY", out wrapModeXY))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid wrapModeXY attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.WrapMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            sampler.WrapModeXY = wrapModeXY;

            CapnpGen.WrapMode wrapModeZ = CapnpGen.WrapMode.clamp;
            if (!TryGetAttribute(samplerNode, "wrapModeZ", out wrapModeZ))
            {
                error = "Sampler node \"" + sampler.Name + "\" does not have a valid wrapModeZ attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.WrapMode));
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

        public static bool ParseBlender(XmlNode blenderNode, List<CapnpGen.Blender> blenders, out string error)
        {
            error = "";
            CapnpGen.Blender blender = new CapnpGen.Blender();

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

            CapnpGen.BlendMode srcBlendMode = CapnpGen.BlendMode.zero;
            if (!TryGetAttribute(blenderNode, "srcBlendMode", out srcBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid srcBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.SrcBlendMode = srcBlendMode;

            CapnpGen.BlendMode destBlendMode = CapnpGen.BlendMode.zero;
            if (!TryGetAttribute(blenderNode, "destBlendMode", out destBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid destBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.DestBlendMode = destBlendMode;

            CapnpGen.BlendOp blendOp = CapnpGen.BlendOp.add;
            if (!TryGetAttribute(blenderNode, "blendOp", out blendOp))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid blendOp attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendOp));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.BlendOp = blendOp;

            CapnpGen.BlendMode srcAlphaBlendMode = CapnpGen.BlendMode.zero;
            if (!TryGetAttribute(blenderNode, "srcAlphaBlendMode", out srcAlphaBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid srcAlphaBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.SrcAlphaBlendMode = srcAlphaBlendMode;

            CapnpGen.BlendMode destAlphaBlendMode = CapnpGen.BlendMode.zero;
            if (!TryGetAttribute(blenderNode, "destAlphaBlendMode", out destAlphaBlendMode))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid destAlphaBlendMode attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendMode));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            blender.DestAlphaBlendMode = destAlphaBlendMode;

            CapnpGen.BlendOp alphaBlendOp = CapnpGen.BlendOp.add;
            if (!TryGetAttribute(blenderNode, "alphaBlendOp", out alphaBlendOp))
            {
                error = "Blender node \"" + blender.Name + "\" does not have a valid alphaBlendOp attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.BlendOp));
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

        public static bool ParseMaterial(XmlNode materialNode, List<CapnpGen.Material> materials, List<CapnpGen.Blender> blenders, List<CapnpGen.Sampler> samplers, out string error)
        {
            error = "";
            CapnpGen.Material material = new CapnpGen.Material();
            material.Descriptor = new CapnpGen.MaterialDescriptor();
            material.Descriptor.Samplers = samplers;

            // Name
            string name = "";
            if (!TryGetAttribute(materialNode, "name", out name))
            {
                error = "Material node does not have a name attribute";
                return false;
            }
            material.Descriptor.Name = name;

            // Parameters
            List<CapnpGen.Parameter> parameters = new List<CapnpGen.Parameter>();
            XmlNodeList parameterNodes = materialNode.SelectNodes("Parameter");
            foreach (XmlNode parameterNode in parameterNodes)
            {
                if (!ParseParameter(parameterNode, parameters, out error))
                {
                    return false;
                }
            }
            material.Descriptor.Parameters = parameters;

            // Inputs
            List<CapnpGen.Input> inputs = new List<CapnpGen.Input>();
            XmlNodeList inputNodes = materialNode.SelectNodes("Input");
            foreach (XmlNode inputNode in inputNodes)
            {
                if (!ParseInput(inputNode, inputs, out error))
                {
                    return false;
                }
            }
            material.Descriptor.Inputs = inputs;

            // Outputs
            List<CapnpGen.Output> outputs = new List<CapnpGen.Output>();
            XmlNodeList outputNodes = materialNode.SelectNodes("Output");
            foreach (XmlNode outputNode in outputNodes)
            {
                if (!ParseOutput(outputNode, outputs, blenders, out error))
                {
                    return false;
                }
            }
            material.Descriptor.Outputs = outputs;

            // PSBody
            XmlNode psBodyNode = materialNode.SelectSingleNode("PSBody");
            if (psBodyNode == null)
            {
                error = "Material node \"" + material.Descriptor.Name + "\" does not have a PSBody node, this is required.";
                return false;
            }
            material.PsBody = psBodyNode.InnerText.Trim();

            materials.Add(material);
            return true;
        }

        static bool ParseParameter(XmlNode parameterNode, List<CapnpGen.Parameter> parameters, out string error)
        {
            error = "";
            CapnpGen.Parameter parameter = new CapnpGen.Parameter();

            string name = "";
            if (!TryGetAttribute(parameterNode, "name", out name))
            {
                error = "Parameter node does not have a name attribute";
                return false;
            }
            parameter.Name = name;

            CapnpGen.ParameterType parameterType = CapnpGen.ParameterType.@float;
            if (!TryGetAttribute(parameterNode, "type", out parameterType))
            {
                error = "Parameter node \"" + parameter.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.ParameterType));
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
                CapnpGen.SubType subType = CapnpGen.SubType.@float;
                if (!TryGetAttribute(parameterNode, "subtype", out subType))
                {
                    error = "Parameter node \"" + parameter.Name + "\" is of a texture type but does not have a valid subtype attribute, valid values: ";
                    var values = Enum.GetValues(typeof(CapnpGen.SubType));
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

        static bool ParseInput(XmlNode inputNode, List<CapnpGen.Input> inputs, out string error)
        {
            error = "";
            CapnpGen.Input input = new CapnpGen.Input();

            string name = "";
            if (!TryGetAttribute(inputNode, "name", out name))
            {
                error = "Input node does not have a name attribute";
                return false;
            }
            input.Name = name;

            CapnpGen.InputType inputType = CapnpGen.InputType.color;
            if (!TryGetAttribute(inputNode, "type", out inputType))
            {
                error = "Input node \"" + input.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.InputType));
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

        static bool ParseOutput(XmlNode outputNode, List<CapnpGen.Output> outputs, List<CapnpGen.Blender> blenders, out string error)
        {
            error = "";
            CapnpGen.Output output = new CapnpGen.Output();

            string name = "";
            if (!TryGetAttribute(outputNode, "name", out name))
            {
                error = "Output node does not have a name attribute";
                return false;
            }
            output.Name = name;

            CapnpGen.OutputType outputType = CapnpGen.OutputType.color;
            if (!TryGetAttribute(outputNode, "type", out outputType))
            {
                error = "Output node \"" + output.Name + "\" does not have a valid type attribute, valid values: ";
                var values = Enum.GetValues(typeof(CapnpGen.OutputType));
                foreach (var value in values)
                {
                    error += value + " ";
                }
                return false;
            }
            output.Type = outputType;

            output.Blender = null;
            // If output type was color we'll need a blender
            if (output.Type == CapnpGen.OutputType.color)
            {
                // Get the blender name
                string blenderName = "";
                if (!TryGetAttribute(outputNode, "blender", out blenderName))
                {
                    error = "Output node \"" + output.Name + "\" is of type Color but does not have a blender attribute. All color outputs need a blender attribute";
                    return false;
                }

                // Find and reference the blender itself
                foreach (CapnpGen.Blender blender in blenders)
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

                CapnpGen.SubType subType = CapnpGen.SubType.@float;
                if (!TryGetAttribute(outputNode, "subtype", out subType))
                {
                    error = "Output node \"" + output.Name + "\" is of type Color but does not have a valid subtype attribute, valid values: ";
                    var values = Enum.GetValues(typeof(CapnpGen.SubType));
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

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.FilterMode value)
        {
            value = CapnpGen.FilterMode.point;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.FilterMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.WrapMode value)
        {
            value = CapnpGen.WrapMode.clamp;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.WrapMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.BlendMode value)
        {
            value = CapnpGen.BlendMode.zero;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.BlendMode>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.BlendOp value)
        {
            value = CapnpGen.BlendOp.add;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.BlendOp>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.ParameterType value)
        {
            value = CapnpGen.ParameterType.@float;

            if (node.Attributes[attribute] == null)
                return false;

            string valueString = node.Attributes[attribute].Value;
            var enumValues = Enum.GetValues(typeof(CapnpGen.ParameterType));
            foreach (var enumValue in enumValues)
            {
                string enumValueString = enumValue.ToString();
                if (enumValueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                {
                    enumValueString = enumValueString.Substring(1);
                }

                if (valueString == enumValueString)
                {
                    value = (CapnpGen.ParameterType)enumValue;
                    return true;
                }
            }

            return false;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.SubType value)
        {
            value = CapnpGen.SubType.@float;

            if (node.Attributes[attribute] == null)
                return false;

            string valueString = node.Attributes[attribute].Value;
            var enumValues = Enum.GetValues(typeof(CapnpGen.SubType));
            foreach (var enumValue in enumValues)
            {
                string enumValueString = enumValue.ToString();
                if (enumValueString[0] == '@') // Capnproto will prepend "@" on reserved keywords, let's filter that away
                {
                    enumValueString = enumValueString.Substring(1);
                }

                if (valueString == enumValueString)
                {
                    value = (CapnpGen.SubType)enumValue;
                    return true;
                }
            }

            return false;
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.InputType value)
        {
            value = CapnpGen.InputType.position;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.InputType>(node.Attributes[attribute].Value, out value);
        }

        static bool TryGetAttribute(XmlNode node, string attribute, out CapnpGen.OutputType value)
        {
            value = CapnpGen.OutputType.color;

            if (node.Attributes[attribute] == null)
                return false;

            return Enum.TryParse<CapnpGen.OutputType>(node.Attributes[attribute].Value, out value);
        }

    }
}