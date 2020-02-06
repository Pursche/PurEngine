using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml;
using System.Reflection;
using System.Linq;
using Vortice.Dxc;
using System.Runtime.InteropServices;
using Capnp;

namespace Cooker.Cookers
{
    static class MaterialCompiler
    {
        public static bool CompileMaterial(CapnpGen.Material material, string outputDirectory, out string error)
        {
            error = "";
            Directory.CreateDirectory(outputDirectory);

            // Load templates
            string vsTemplate = "";
            if (!GetEmbeddedResource("templateVS.hlsl", out error, out vsTemplate))
            {
                return false;
            }

            string psTemplate = "";
            if (!GetEmbeddedResource("templatePS.hlsl", out error, out psTemplate))
            {
                return false;
            }

            // Mutate the templates
            string vs = MutateVS(material, vsTemplate);
            string ps = MutatePS(material, psTemplate);

            // Compile the shaders
            DxcCompilerOptions options = new DxcCompilerOptions();
            options.ShaderModel = DxcShaderModel.Model6_4;
            options.EnableDebugInfo = true;
            options.OptimizationLevel = 0;

            IDxcBlob vsBlob;
            if (!CompileShader(vs, material.Descriptor.Name, DxcShaderStage.VertexShader, options, out vsBlob, out error))
            {
                return false;
            }

            IDxcBlob psBlob;
            if (!CompileShader(ps, material.Descriptor.Name, DxcShaderStage.PixelShader, options, out psBlob, out error))
            {
                return false;
            }

            // Save the shader files
            string vsPath = Path.Combine(outputDirectory, material.Descriptor.Name + ".vs.hlsl.cso");
            byte[] vsBytes = new byte[vsBlob.GetBufferSize()];
            unsafe
            {
                Marshal.Copy((IntPtr)vsBlob.GetBufferPointer(), vsBytes, 0, (int)vsBlob.GetBufferSize());
            }
            File.WriteAllBytes(vsPath, vsBytes);

            string psPath = Path.Combine(outputDirectory, material.Descriptor.Name + ".ps.hlsl.cso");
            byte[] psBytes = new byte[psBlob.GetBufferSize()];
            unsafe
            {
                Marshal.Copy((IntPtr)psBlob.GetBufferPointer(), psBytes, 0, (int)psBlob.GetBufferSize());
            }
            File.WriteAllBytes(psPath, psBytes);

            // Serialize and save the material descriptor
            string descriptorPath = Path.Combine(outputDirectory, material.Descriptor.Name + ".material.bin");
            using (FileStream outStream = new FileStream(descriptorPath, FileMode.Create))
            {
                MessageBuilder message = MessageBuilder.Create();
                var root = message.BuildRoot<CapnpGen.MaterialDescriptor.WRITER>();
                material.Descriptor.serialize(root);

                var pump = new FramePump(outStream);
                pump.Send(message.Frame);
            }

            return true;
        }

        static bool GetEmbeddedResource(string embeddedFilename, out string error, out string result)
        {
            error = "";
            result = "";

            try
            {
                Assembly assembly = Assembly.GetExecutingAssembly();
                string[] resourceNames = assembly.GetManifestResourceNames();
                string resourceName = resourceNames.Single(str => str.EndsWith(embeddedFilename));

                using (Stream stream = assembly.GetManifestResourceStream(resourceName))
                using (StreamReader reader = new StreamReader(stream))
                {
                    result = reader.ReadToEnd();
                    return true;
                }
            }
            catch(Exception e)
            {
                error = e.Message;
            }
            
            return false;
        }

        static string MutateVS(CapnpGen.Material material, string vsTemplate)
        {
            // Generate VSINPUT block for the template
            string vsInput = GenerateVSInput(material);

            // Generate VSOUTPUT block for the template
            string vsOutput = GenerateVSOutput(material);

            // Generate VSBODY block for the template
            string vsBody = GenerateVSBody(material);

            // Mutate the template
            vsTemplate = vsTemplate.Replace("${VSINPUT}", vsInput);
            vsTemplate = vsTemplate.Replace("${VSOUTPUT}", vsOutput);
            vsTemplate = vsTemplate.Replace("${VSBODY}", vsBody);

            return vsTemplate;
        }

        static string GenerateVSInput(CapnpGen.Material material)
        {
            string vsInputs = "";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.position))
                vsInputs += "float3 pos : POSITION;\n";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.color))
                vsInputs += "float4 color : COLOR;\n";

            return vsInputs;
        }

        static string GenerateVSOutput(CapnpGen.Material material)
        {
            string vsOutputs = "";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.position))
                vsOutputs += "float4 pos : SV_POSITION;\n";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.color))
                vsOutputs += "float4 color : COLOR;\n";

            return vsOutputs;
        }

        static string GenerateVSBody(CapnpGen.Material material)
        {
            string vsBody = "";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.position))
                vsBody += "output.pos = mul(mul(mul(float4(input.pos, 1.0f), modelCB.model), viewCB.view), viewCB.proj);\n";

            if (material.Descriptor.Inputs.Any(input => input.Type == CapnpGen.InputType.color))
                vsBody += "output.color = input.color;\n";

            return vsBody;
        }

        static string MutatePS(CapnpGen.Material material, string psTemplate)
        {
            // Generate PSINPUT block for the template
            string psInput = GeneratePSInput(material);

            // Generate PSOUTPUT block for the template
            string psOutput = GeneratePSOutput(material);

            // Generate PSPARAMETERS block for the template
            string psParameters = GeneratePSParameters(material);

            // Before we mutate, we need to replace all references to CB parameters, for example X, with materialCB.X
            //SolveCBVariableNames(material);

            // Mutate the template
            psTemplate = psTemplate.Replace("${PSINPUT}", psInput);
            psTemplate = psTemplate.Replace("${PSOUTPUT}", psOutput);
            psTemplate = psTemplate.Replace("${PSPARAMETERS}", psParameters);
            psTemplate = psTemplate.Replace("${PSBODY}", material.PsBody);

            return psTemplate;
        }

        static string GeneratePSInput(CapnpGen.Material material)
        {
            string psInput = "";

            // Vertex position
            foreach(CapnpGen.Input input in material.Descriptor.Inputs)
            {
                if (input.Type == CapnpGen.InputType.position)
                {
                    psInput += "float4 " + input.Name + " : SV_Position;\n";
                    break;
                }
            }

            // Vertex Color
            foreach (CapnpGen.Input input in material.Descriptor.Inputs)
            {
                if (input.Type == CapnpGen.InputType.color)
                {
                    psInput += "float4 " + input.Name + " : COLOR;\n";
                    break;
                }
            }

            // PrimitiveID
            foreach (CapnpGen.Input input in material.Descriptor.Inputs)
            {
                if (input.Type == CapnpGen.InputType.primitiveID)
                {
                    psInput += "uint " + input.Name + " : SV_PrimitiveID;\n";
                    break;
                }
            }

            return psInput;
        }

        static string GetOutputType(CapnpGen.Output output)
        {
            string outputType = "";

            switch(output.Type)
            {
                case CapnpGen.OutputType.color:
                    outputType = output.SubType.ToString();

                    // Capnproto will prepend "@" on reserved keywords, let's filter that away
                    if (outputType[0] == '@') 
                        outputType = outputType.Substring(1);

                    break;
                case CapnpGen.OutputType.depth:
                    outputType = "float";
                    break;
            }

            return outputType;
        }

        static string GeneratePSOutput(CapnpGen.Material material)
        {
            string psOutput = "";

            int numColorTargets = 0;
            int numDepthTargets = 0;
            foreach (CapnpGen.Output output in material.Descriptor.Outputs)
            {
                string type = GetOutputType(output);

                psOutput += type + " " + output.Name + " : ";

                if (output.Type == CapnpGen.OutputType.color)
                {
                    psOutput += "SV_Target" + numColorTargets + ";\n";
                    numColorTargets++;
                }
                else if (output.Type == CapnpGen.OutputType.depth)
                {
                    psOutput += "SV_Depth" + numDepthTargets + ";\n";
                    numDepthTargets++;
                }
            }

            return psOutput;
        }

        static string GeneratePSParameters(CapnpGen.Material material)
        {
            string psParameters = "";

            // First lets generate the texture block
            int numTextures = 0;
            foreach(CapnpGen.Parameter parameter in material.Descriptor.Parameters)
            {
                string type = parameter.Type.ToString();
                if (type.StartsWith("texture"))
                {
                    type = char.ToUpper(type[0]) + type.Substring(1);
                    string subType = parameter.SubType.ToString();

                    psParameters += type + "<" + subType + "> " + parameter.Name + " : register(t" + numTextures + ");\n";
                }
            }

            // Then lets generate the sampler block
            int numSamplers = 0;
            foreach (CapnpGen.Sampler sampler in material.Descriptor.Samplers)
            {
                psParameters += "SamplerState " + sampler.Name + " : register(s" + numSamplers + ");\n";
            }

            // Lastly lets generate the constant buffer block
            string cbBlock = "";
            foreach (CapnpGen.Parameter parameter in material.Descriptor.Parameters)
            {
                string type = parameter.Type.ToString();
                if (!type.StartsWith("texture"))
                {
                    // Capnproto will prepend "@" on reserved keywords, let's filter that away
                    if (type[0] == '@')
                        type = type.Substring(1);

                    // In our shaders we treat our color type as a float4, the color destinction is only necessary for the binder
                    if (type == "color")
                        type = "float4";

                    cbBlock += type + " " + parameter.Name + ";\n";
                }
            }
            if (cbBlock.Length > 0)
            {
                psParameters += "cbuffer materialCB : register(b2)\n{\n";
                psParameters += cbBlock;
                psParameters += "};\n";
            }

            return psParameters;
        }

        static void SolveCBVariableNames(CapnpGen.Material material)
        {
            // First find all CB parameter names
            List<string> parameterNames = new List<string>();
            foreach (CapnpGen.Parameter parameter in material.Descriptor.Parameters)
            {
                if (!parameter.Type.ToString().StartsWith("texture"))
                {
                    parameterNames.Add(parameter.Name);
                }
            }

            // Then replace all usages of NAME with materialCB.NAME
            foreach(string parameterName in parameterNames)
            {
                material.PsBody = material.PsBody.Replace(parameterName, "materialCB." + parameterName);
            }
        }

        static bool CompileShader(string shader, string shaderName, DxcShaderStage stage, DxcCompilerOptions options, out IDxcBlob blob, out string error)
        {
            NativeLibraryLoader.NativeLibrary lib = new NativeLibraryLoader.NativeLibrary("dxil.dll");

            error = "";
            blob = null;
            IDxcOperationResult result = DxcCompiler.Compile(stage, shader, "main", shaderName, options);

            if (result.GetStatus() != 0)
            {
                unsafe
                {
                    IntPtr errorPtr = (IntPtr)result.GetErrors().GetBufferPointer();
                    error = Marshal.PtrToStringAnsi(errorPtr);
                }

                return false;
            }

            blob = result.GetResult();
            return true;
        }
    }
}