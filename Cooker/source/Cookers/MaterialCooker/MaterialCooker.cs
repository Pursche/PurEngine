using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml;
using Capnp;

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
            List<CapnpGen.CapSampler> samplers = new List<CapnpGen.CapSampler>();
            XmlNodeList samplerNodes = materialFileNode.SelectNodes("Sampler");
            foreach(XmlNode samplerNode in samplerNodes)
            {
                if (!MaterialParser.ParseSampler(samplerNode, samplers, out error))
                {
                    return false;
                }
            }

            // Parse all blenders
            List<CapnpGen.CapBlender> blenders = new List<CapnpGen.CapBlender>();
            XmlNodeList blenderNodes = materialFileNode.SelectNodes("Blender");
            foreach (XmlNode blenderNode in blenderNodes)
            {
                if (!MaterialParser.ParseBlender(blenderNode, blenders, out error))
                {
                    return false;
                }
            }

            // Parse all materials
            List<CapnpGen.CapMaterial> materials = new List<CapnpGen.CapMaterial>();
            XmlNodeList materialNodes = materialFileNode.SelectNodes("Material");
            foreach(XmlNode materialNode in materialNodes)
            {
                if (!MaterialParser.ParseMaterial(materialNode, materials, blenders, samplers, out error))
                {
                    return false;
                }
            }

            // Parse all material instances
            List<CapnpGen.CapMaterialInstance> materialInstances = new List<CapnpGen.CapMaterialInstance>();
            XmlNodeList materialInstanceNodes = materialFileNode.SelectNodes("MaterialInstance");
            foreach(XmlNode materialInstanceNode in materialInstanceNodes)
            {
                if (!MaterialParser.ParseMaterialInstance(materialInstanceNode, materialInstances, materials, materialPath, out error))
                {
                    return false;
                }
            }

            // Compile all materials
            foreach(CapnpGen.CapMaterial material in materials)
            {
                if (!MaterialCompiler.CompileMaterial(material, outputDirectory, out error))
                {
                    return false;
                }
            }

            // Serialize and save the all material instance headers
            foreach(CapnpGen.CapMaterialInstance materialInstance in materialInstances)
            {
                string headerPath = Path.Combine(outputDirectory, materialInstance.Header.Name + ".material");
                Directory.CreateDirectory(Path.GetDirectoryName(headerPath));
                using (FileStream outStream = new FileStream(headerPath, FileMode.Create))
                {
                    MessageBuilder message = MessageBuilder.Create();
                    var root = message.BuildRoot<CapnpGen.CapMaterialInstanceHeader.WRITER>();
                    materialInstance.Header.serialize(root);

                    var pump = new FramePump(outStream);
                    pump.Send(message.Frame);
                }

                // Copy all textures used by the material instance into the Data/material folder
                foreach(CapnpGen.CapMaterialTexture texture in materialInstance.Header.Textures)
                {
                    string destPath = Path.Combine(outputDirectory, texture.Path);
                    Directory.CreateDirectory(Path.GetDirectoryName(destPath));
                    File.Copy(texture.ContentPath, destPath, true);
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