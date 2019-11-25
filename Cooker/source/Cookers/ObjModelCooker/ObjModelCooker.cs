using Capnp;
using Neo.IronLua;
using ObjLoader.Loader.Data.Elements;
using ObjLoader.Loader.Data.VertexData;
using ObjLoader.Loader.Loaders;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

namespace Cooker.Cookers
{
    class ObjModelCooker : BaseCooker
    {
        public class CustomMaterialStreamProvider : IMaterialStreamProvider
        {
            public void SetAssetPath(string assetPath)
            {
                this.assetDir = Path.GetFullPath(Path.Combine(assetPath, @"../")); ;
            }

            public Stream Open(string materialName)
            {
                string materialFilePath = Path.Combine(assetDir, materialName);

                return File.Open(materialFilePath, FileMode.Open, FileAccess.Read);
            }

            string assetDir;
        }

        public override void Init()
        {
            objLoaderFactory = new ObjLoaderFactory();
            materialStreamProvider = new CustomMaterialStreamProvider();
            objLoader = objLoaderFactory.Create(materialStreamProvider);
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
            string assetPath = GetAssetPath(luaPath);

            if (!assetPath.EndsWith(".obj"))
            {
                error = "We only convert .obj files";
                return false;
            }

            return File.Exists(assetPath);
        }

        public override bool Cook(string luaPath, LuaGlobal environment, string outputDirectory, string intermediateDirectory, out string producedFile, out string error)
        {
            error = "";
            producedFile = "";
            string assetPath = GetAssetPath(luaPath);
            materialStreamProvider.SetAssetPath(assetPath);

            using (FileStream assetStream = new FileStream(assetPath, FileMode.Open))
            {
                LoadResult result = objLoader.Load(assetStream);

                CapnpGen.CapModel model = new CapnpGen.CapModel();

                // Convert vertices
                List<CapnpGen.CapVector3> vertexPositions = new List<CapnpGen.CapVector3>();
                foreach (Vertex vertex in result.Vertices)
                {
                    CapnpGen.CapVector3 pos = new CapnpGen.CapVector3();
                    pos.X = vertex.X;
                    pos.Y = vertex.Y;
                    pos.Z = vertex.Z;

                    vertexPositions.Add(pos);
                }
                model.VertexPositions = vertexPositions;

                // Convert indices
                List<UInt32> indices = new List<UInt32>();
                foreach (ObjLoader.Loader.Data.Elements.Group group in result.Groups) // A group represents a "submesh"
                {
                    foreach (Face face in group.Faces)
                    {
                        Debug.Assert(face.Count == 4); // I haven't seen a model where faces don't have 4 indices yet

                        // We split the face (quad) into two triangles, the first one with index 0 1 and 2
                        indices.Add((UInt32)(face[0].VertexIndex - 1));
                        indices.Add((UInt32)(face[1].VertexIndex - 1));
                        indices.Add((UInt32)(face[2].VertexIndex - 1));

                        // The second one with index 0 2 and 3
                        indices.Add((UInt32)(face[2].VertexIndex - 1));
                        indices.Add((UInt32)(face[3].VertexIndex - 1));
                        indices.Add((UInt32)(face[0].VertexIndex - 1));
                    }
                }
                model.Indices = indices;

                // Create output file
                string outputFileName = Path.ChangeExtension(Path.GetFileName(assetPath), ".PurModel");
                producedFile = Path.Combine(outputDirectory, "models", outputFileName);
                Directory.CreateDirectory(Path.GetDirectoryName(producedFile));
                using (FileStream outStream = new FileStream(producedFile, FileMode.Create))
                {
                    MessageBuilder message = MessageBuilder.Create();
                    var root = message.BuildRoot<CapnpGen.CapModel.WRITER>();
                    model.serialize(root);

                    var pump = new FramePump(outStream);
                    pump.Send(message.Frame);
                }
            }

            return true;
        }

        ObjLoaderFactory objLoaderFactory;
        CustomMaterialStreamProvider materialStreamProvider;
        IObjLoader objLoader;
    }
}