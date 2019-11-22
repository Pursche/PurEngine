using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

namespace Cooker.Cookers
{
    class ObjModelCooker : BaseCooker
    {
        public override void Init()
        {

        }

        public override void RegisterFunctions(LuaGlobal environment)
        {
            /*mutations = new Dictionary<string, string>();
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
            });*/
        }

        public override bool CanCook(string luaPath, LuaGlobal environment, out string error)
        {
            error = "";
            string shaderPath = GetAssetPath(luaPath);

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
            string shaderPath = GetAssetPath(luaPath);

            return false;
        }

    }
}