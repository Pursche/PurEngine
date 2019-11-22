using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

namespace Cooker.Cookers
{
    class ShaderCooker : BaseCooker
    {
        enum ShaderType
        {
            Vertex,
            Pixel,
            Compute,
            Unknown
        }

        Dictionary<string, ShaderType> validShaderTypes = new Dictionary<string, ShaderType>
        {
            {"vs", ShaderType.Vertex },
            {"ps", ShaderType.Pixel },
            {"cs", ShaderType.Compute }
        };

        string[] ignoreWarnings = new string[]
        {
            //"DXIL.dll not found"
        };

        public class ShaderCompileSettings
        {
            public string shaderPath;
            public string outputDirectory;
            public string intermediateDirectory;
            
            public string entryPoint;
            public string optimization;
            public string profile;
            public bool debug;
            public string outputFile;
            public string[] defines;

            public LuaTable ToLuaTable()
            {
                dynamic table = new LuaTable();
                table.shaderPath = shaderPath;
                table.outputDirectory = outputDirectory;
                table.intermediateDirectory = intermediateDirectory;

                table.entryPoint = entryPoint;
                table.optimization = optimization;
                table.profile = profile;
                table.debug = debug;
                table.outputFile = outputFile;

                LuaTable definesTable = new LuaTable();

                for(int i = 0; i < defines.Length; i++)
                {
                    definesTable.Add(defines[i]);
                }

                table.defines = definesTable;

                return table;
            }

            static public ShaderCompileSettings FromLuaTable(dynamic table)
            {
                ShaderCompileSettings settings = new ShaderCompileSettings();

                settings.shaderPath = table.shaderPath;
                settings.outputDirectory = table.outputDirectory;
                settings.intermediateDirectory = table.intermediateDirectory;

                settings.entryPoint = table.entryPoint;
                settings.optimization = table.optimization;
                settings.profile = table.profile;
                settings.debug = table.debug;
                settings.outputFile = table.outputFile;

                LuaTable luaDefines = (LuaTable)table.defines;
                settings.defines = new string[luaDefines.ArrayList.Count];
                
                for(int i = 0; i < luaDefines.ArrayList.Count; i++)
                {
                    settings.defines[i] = (string)luaDefines.ArrayList[i];
                }

                return settings;
            }
        }

        public override void Init()
        {

        }

        Dictionary<string, string> mutations;
        public override void RegisterFunctions(LuaGlobal environment)
        {
            mutations = new Dictionary<string, string>();
            dynamic dynamicEnvironment = environment;

            dynamicEnvironment.setMutation = new Action<string, object>((mutationToken, value) =>
            {
                if (mutations.ContainsKey(mutationToken))
                {
                    mutations[mutationToken] = value.ToString();
                }
                else
                {
                    mutations.Add(mutationToken, value.ToString());
                }
            });
        }

        public override bool CanCook(string luaPath, LuaGlobal environment, out string error)
        {
            error = "";
            string shaderPath = GetShaderPath(luaPath, environment);

            if (!File.Exists(shaderPath))
            {
                error = "Asset does not exist";
                return false;
            }

            if (!shaderPath.EndsWith(".hlsl"))
            {
                error = "We only convert .hlsl shader files";
                return false;
            }

            return true;
        }

        public override bool Cook(string luaPath, LuaGlobal environment, string outputDirectory, string intermediateDirectory, out string producedFile, out string error)
        {
            error = "";
            producedFile = "";
            string shaderPath = GetShaderPath(luaPath, environment);

            // Figure out which shader type we're trying to compile
            ShaderType shaderType = GetShaderType(shaderPath, environment);
            if (shaderType == ShaderType.Unknown)
            {
                error = "Shader type was unknown, either use one of the following file extensions: ";

                bool first = true;
                foreach (KeyValuePair<string, ShaderType> validShaderType in validShaderTypes)
                {
                    if (!first)
                    {
                        error += ", ";
                    }
                    else
                        first = false;

                    error += validShaderType.Key + ".hlsl";
                }
                error += "\n OR set shaderType in the .lua file";

                return false;
            }

            // Try to get all settings from .lua file or acceptable defaults
            ShaderCompileSettings compileSettings = new ShaderCompileSettings();
            compileSettings.shaderPath = shaderPath;
            compileSettings.outputDirectory = Path.Combine(outputDirectory, "shaders");
            compileSettings.intermediateDirectory = intermediateDirectory;

            compileSettings.entryPoint = Env.TryGetDefault<string>(environment, "entryPoint", "main");
            compileSettings.optimization = Env.TryGetDefault<string>(environment, "optimization", "Od");
            compileSettings.profile = GetShaderProfile(shaderType, environment);
            compileSettings.debug = Env.TryGetDefault<bool>(environment, "debug", true);
            compileSettings.outputFile = GetShaderOutputFileName(shaderPath, environment, shaderType);
            compileSettings.defines = Env.TryGetArrayDefault<string>(environment, "defines", new string[0]);

            if (environment.ContainsMember("OnCompile"))
            {
                string producedFileStr = "";

                // Add the compileShader function to the lua environment
                dynamic dynamicEnvironment = environment;
                dynamicEnvironment.compileShader = new Func<LuaTable, bool>((compileSettingsTable) =>
                {
                    ShaderCompileSettings settings = ShaderCompileSettings.FromLuaTable(compileSettingsTable);
                    producedFileStr = Path.Combine(settings.outputDirectory, settings.outputFile);
                    return CompileShader(settings);
                });

                // Lets call OnCompile to allow the lua file to handle mutations on its own
                LuaTable table = compileSettings.ToLuaTable();
                LuaResult result = environment.CallMember("OnCompile", table);

                // Remove the compileShader function
                dynamicEnvironment.compileShader = null;

                if (result.Count != 1)
                {
                    Debug.Fail("[SHADER COOKER]: If a Lua file overrides OnCompile it has to return a bool to specify if it succeeded");
                }
                producedFile = producedFileStr;
                return (bool)result[0];

            }
            else
            {
                producedFile = Path.Combine(compileSettings.outputDirectory, compileSettings.outputFile);
                return CompileShader(compileSettings);
            }
        }

        string GetShaderOutputFileName(string shaderPath, LuaGlobal environment, ShaderType shaderType)
        {
            string name = Path.GetFileName(shaderPath) + ".cso";

            string overrideName = "";
            if (Env.TryGet<string>(environment, "outputFile", out overrideName))
            {
                name = overrideName;
                foreach (KeyValuePair<string, ShaderType> validShaderType in validShaderTypes)
                {
                    if (shaderType == validShaderType.Value)
                    {
                        name = Path.ChangeExtension(name, validShaderType.Key + ".cso");
                        break;
                    }
                }
            }
            return name;
        }

        string GetShaderPath(string luaPath, LuaGlobal environment)
        {
            string shaderPath = GetAssetPath(luaPath);

            string shaderFile = "";
            if (Env.TryGet<string>(environment, "shaderFile", out shaderFile))
            {
                shaderPath = Path.Combine(Path.GetDirectoryName(luaPath), shaderFile);
            }
            return shaderPath;
        }

        string GetShaderProfile(ShaderType shaderType, LuaGlobal environment)
        {
            string profile = "";
            if (!Env.TryGet<string>(environment, "profile", out profile))
            {
                foreach (KeyValuePair<string, ShaderType> validShaderType in validShaderTypes)
                {
                    if (shaderType == validShaderType.Value)
                    {
                        profile = validShaderType.Key + "_6_5";
                        break;
                    }
                }
            }
            Debug.Assert(profile != ""); // We already checked for this, but let's verify to avoid future mistakes

            return profile;
        }

        private ShaderType GetShaderType(string shaderPath, LuaGlobal environment)
        {
            // First try getting shader type from filename
            foreach(KeyValuePair<string, ShaderType> validShaderType in validShaderTypes)
            {
                string validExtension = validShaderType.Key + ".hlsl";

                if (shaderPath.EndsWith(validExtension))
                {
                    return validShaderType.Value;
                }
            }

            // If it didnt work, check the environment for a defined shader type
            string shaderTypeStr;
            if (Env.TryGet<string>(environment, "shaderType", out shaderTypeStr))
            {
                foreach (KeyValuePair<string, ShaderType> validShaderType in validShaderTypes)
                {
                    if (shaderTypeStr == validShaderType.Key)
                    {
                        return validShaderType.Value;
                    }
                }
            }
            
            // Otherwise we have no idea what shadertype this is...
            return ShaderType.Unknown;
        }

        Regex tokenRegex = new Regex("\\$\\{(.*?)}"); // ${...} Checks for this kind of token
        bool MutateLine(ref string line, out string error)
        {
            error = "";

            // Check for any mutation tokens
            MatchCollection matches = tokenRegex.Matches(line);
            if (matches.Count > 0)
            {
                foreach (Match match in matches)
                {
                    string fullToken = match.Groups[0].Value;
                    string innerToken = match.Groups[1].Value;

                    if (mutations.ContainsKey(innerToken))
                    {
                        line = line.Replace(fullToken, mutations[innerToken]);
                    }
                    else
                    {
                        error = "Unmatched mutation token found in shader: " + fullToken;
                        return false;
                    }
                }
            }

            return true;
        }

        bool MutateShader(string shaderPath, string mutatedShaderPath, out string error)
        {
            error = "";
            using (TextWriter writer = new StreamWriter(mutatedShaderPath, false))
            {
                using (TextReader reader = new StreamReader(shaderPath))
                {
                    int lineCount = 0;
                    string line;
                    while ((line = reader.ReadLine()) != null)
                    {
                        if (!MutateLine(ref line, out error))
                        {
                            error += " - " + shaderPath + " (Line " + lineCount + ")";
                            return false;
                        }
                        writer.WriteLine(line);
                        lineCount++;
                    }
                }
            }

            return true;
        }

        public bool CompileShader(ShaderCompileSettings settings)
        {
            Directory.CreateDirectory(settings.outputDirectory);

            // Mutate shader to intermediate directory
            string mutatedShaderPath = Path.ChangeExtension(Path.Combine(settings.intermediateDirectory, settings.outputFile), ".mut");
            string error = "";
            if (!MutateShader(settings.shaderPath, mutatedShaderPath, out error))
            {
                Debug.WriteLine("[MUTATION ERROR]: " + error);
                return false;
            }

            // Create the commandline arguments
            string compilerPath = Path.GetFullPath(Path.Combine(settings.intermediateDirectory, "../../External/dxc/bin/dxc.exe")); ;

            string commandLine = "";
            commandLine += " -T " + settings.profile;
            commandLine += " -E " + settings.entryPoint;
            commandLine += " -" + settings.optimization;
            if (settings.debug)
                commandLine += " -Zi -Qembed_debug";
            commandLine += " -Fo \"" + Path.Combine(settings.outputDirectory, settings.outputFile) + "\"";
            foreach(string define in settings.defines)
            {
                commandLine += " -D " + define;
            }
            commandLine += " -I \"" + Path.GetFullPath(Path.GetDirectoryName(settings.shaderPath)) + "\"";

            commandLine += " \"" + mutatedShaderPath + "\"";

            //Console.WriteLine("Command: " + compilerPath + " " + commandLine);

            // TODO: MAYBE we should use DXClib, but the current wrappers don't support the experimental features I want to use
            Process process = new Process();
            process.StartInfo.FileName = compilerPath;
            process.StartInfo.Arguments = commandLine;
            process.StartInfo.WorkingDirectory = settings.intermediateDirectory;
            process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardInput = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.CreateNoWindow = true;

            process.Start();

            System.Text.StringBuilder outputStr = new System.Text.StringBuilder();

            process.OutputDataReceived += (sender, e) =>
            {
                if (e.Data != null)
                {
                    Console.WriteLine(e.Data);
                }
            };

            System.Text.StringBuilder errorStr = new System.Text.StringBuilder();
            System.Text.StringBuilder warningStr = new System.Text.StringBuilder();
            process.ErrorDataReceived += (sender, e) =>
            {
                if (e.Data != null && e.Data.Length > 0)
                {
                    if (e.Data.Contains("warning"))
                    {
                        bool ignore = false;
                        foreach(string ignoreWarning in ignoreWarnings)
                        {
                            if (e.Data.Contains(ignoreWarning))
                            {
                                ignore = true;
                                break;
                            }
                        }

                        if (!ignore)
                            warningStr.AppendLine(e.Data);
                    }
                    else
                    {
                        errorStr.AppendLine(e.Data);
                    }
                }
            };
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();

            process.WaitForExit();

            if (warningStr.Length > 0)
            {
                Console.WriteLine(warningStr);
            }

            if (errorStr.Length > 0)
            {
                Console.WriteLine("[Shader Compiler Error] " + errorStr.ToString());
                return false;
            }

            return true;
        }


    }
}