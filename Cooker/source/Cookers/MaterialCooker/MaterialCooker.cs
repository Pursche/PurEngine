using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml;

namespace Cooker.Cookers
{
    class MaterialCooker : BaseCooker
    {
        public override void Init()
        {

        }

        public override void RegisterFunctions(LuaGlobal environment)
        {
            
        }

        public override bool CanCook(string luaPath, LuaGlobal environment, out string error)
        {
            error = "";

            if (!luaPath.EndsWith(".material.lua"))
            {
                error = "We only convert .material.lua files";
                return false;
            }

            string materialPath = GetMaterialPath(luaPath, environment);
            if (!File.Exists(materialPath))
            {
                error = materialPath + " does not exist!";
                return false;
            }


            return true;
        }

        public override bool Cook(string luaPath, LuaGlobal environment, string outputDirectory, string intermediateDirectory, out string producedFile, out string error)
        {
            error = "";
            producedFile = "";

            outputDirectory = Path.Combine(outputDirectory, "materials");
            Directory.CreateDirectory(outputDirectory);

            // Load XML document
            string materialPath = GetMaterialPath(luaPath, environment);
            XmlDocument xmlDoc = new XmlDocument();
            try
            {
                xmlDoc.Load(materialPath);
            }
            catch(Exception e)
            {
                error = e.Message;
                return false;
            }
            

            // Get the root node
            XmlNode materialFileNode = xmlDoc.SelectSingleNode("//MaterialFile");
            if (materialFileNode == null)
            {
                error = "Material file does not have a root node called MaterialFile";
                return false;
            }

            // Parse all samplers
            List<CapnpGen.Sampler> samplers = new List<CapnpGen.Sampler>();
            XmlNodeList samplerNodes = materialFileNode.SelectNodes("Sampler");
            foreach(XmlNode samplerNode in samplerNodes)
            {
                if (!MaterialParser.ParseSampler(samplerNode, samplers, out error))
                {
                    return false;
                }
            }

            // Parse all blenders
            List<CapnpGen.Blender> blenders = new List<CapnpGen.Blender>();
            XmlNodeList blenderNodes = materialFileNode.SelectNodes("Blender");
            foreach (XmlNode blenderNode in blenderNodes)
            {
                if (!MaterialParser.ParseBlender(blenderNode, blenders, out error))
                {
                    return false;
                }
            }

            // Parse all materials
            List<CapnpGen.Material> materials = new List<CapnpGen.Material>();
            XmlNodeList materialNodes = materialFileNode.SelectNodes("Material");
            foreach(XmlNode materialNode in materialNodes)
            {
                if (!MaterialParser.ParseMaterial(materialNode, materials, blenders, samplers, out error))
                {
                    return false;
                }
            }

            // Compile all materials
            foreach(CapnpGen.Material material in materials)
            {
                if (!MaterialCompiler.CompileMaterial(material, outputDirectory, out error))
                {
                    return false;
                }
            }

            return true;
        }

        string GetMaterialPath(string luaPath, LuaGlobal environment)
        {
            string xmlPath = GetAssetPath(luaPath);

            string xmlFile = "";
            if (Env.TryGet<string>(environment, "material", out xmlFile))
            {
                xmlPath = Path.Combine(Path.GetDirectoryName(luaPath), xmlFile);
            }
            return xmlPath;
        }
    }
}