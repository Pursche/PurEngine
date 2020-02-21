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

                // Get all vertex positions
                List<CapnpGen.CapVector3> vertexPositions = new List<CapnpGen.CapVector3>();
                foreach (Vertex vertex in result.Vertices)
                {
                    CapnpGen.CapVector3 pos = new CapnpGen.CapVector3();
                    pos.X = vertex.X;
                    pos.Y = vertex.Y;
                    pos.Z = vertex.Z;

                    vertexPositions.Add(pos);
                }
                //model.VertexPositions = vertexPositions;

                // Get all vertex texCoords
                List<CapnpGen.CapVector2> vertexTexCoords = new List<CapnpGen.CapVector2>();
                foreach (Texture texture in result.Textures)
                {
                    CapnpGen.CapVector2 texCoord = new CapnpGen.CapVector2();
                    texCoord.X = texture.X;
                    texCoord.Y = texture.Y;

                    vertexTexCoords.Add(texCoord);
                }

                // Get all vertex normals
                List<CapnpGen.CapVector3> vertexNormals = new List<CapnpGen.CapVector3>();
                foreach (Normal normal in result.Normals)
                {
                    CapnpGen.CapVector3 vertexNormal = new CapnpGen.CapVector3();
                    vertexNormal.X = normal.X;
                    vertexNormal.Y = normal.Y;
                    vertexNormal.Z = normal.Z;

                    vertexNormals.Add(vertexNormal);
                }

                // Because .obj stores vertex positions, vertex texcoords and vertex normals separately and without duplication we need to "unpack" combined vertices from this data
                // Each .obj model has a list of "groups" which represent submeshes
                // Each group has a list of faces which represent quads, they have 3-4 "indices" that point to vertex positions, vertex texcoords and vertex normals separately
                // We need to iterate over these, and then build one combined vertex for each unique combination of position, texcoord and normal

                // Convert indices

                // This dictionary will hold unique vertices as keys, and it's corresponding index as value, this makes it easy for us to look up indices of duplicated vertices
                Dictionary<CapnpGen.CapVertex, UInt32> combinedVertices = new Dictionary<CapnpGen.CapVertex, uint>();

                List<UInt32> indices = new List<UInt32>();
                foreach (ObjLoader.Loader.Data.Elements.Group group in result.Groups) // A group represents a "submesh"
                {
                    foreach (Face face in group.Faces)
                    {
                        UInt32[] combinedIndices = new UInt32[face.Count];

                        for (int i = 0; i < face.Count; i++)
                        {
                            CapnpGen.CapVector3 position = vertexPositions[face[i].VertexIndex - 1];

                            int normalIndex = face[i].NormalIndex;
                            CapnpGen.CapVector3 normal = new CapnpGen.CapVector3();
                            normal.Y = 1;
                            if (normalIndex > 0)
                                normal = vertexNormals[normalIndex - 1];

                            int texCoordIndex = face[i].TextureIndex;

                            CapnpGen.CapVector2 texCoord = new CapnpGen.CapVector2();

                            if (texCoordIndex > 0)
                                texCoord = vertexTexCoords[texCoordIndex - 1];

                            combinedIndices[i] = CombineVertex(position, normal, texCoord, ref combinedVertices);
                        }

                        if (face.Count == 4)
                        {
                            // We split the face (quad) into two triangles, the first one with index 0 1 and 2
                            indices.Add(combinedIndices[0]);
                            indices.Add(combinedIndices[1]);
                            indices.Add(combinedIndices[2]);

                            // The second one with index 2 3 and 0
                            indices.Add(combinedIndices[2]);
                            indices.Add(combinedIndices[3]);
                            indices.Add(combinedIndices[0]);
                        }
                        else if (face.Count == 3)
                        {
                            // This kind of face only has one triangle, so it's simple
                            indices.Add(combinedIndices[0]);
                            indices.Add(combinedIndices[1]);
                            indices.Add(combinedIndices[2]);
                        }
                        else
                        {
                            Debug.Assert(false); // I haven't seen a model where faces don't have 4 or 3 indices yet
                        }
                    }
                }

                CapnpGen.CapVertex[] vertices = new CapnpGen.CapVertex[combinedVertices.Count];
                combinedVertices.Keys.CopyTo(vertices, 0);
                model.Vertices = vertices;
                model.Indices = indices;

                // Create output file
                string outputFileName = Path.ChangeExtension(Path.GetFileName(assetPath), ".model");
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

        UInt32 CombineVertex(CapnpGen.CapVector3 position, CapnpGen.CapVector3 normal, CapnpGen.CapVector2 texCoord, ref Dictionary<CapnpGen.CapVertex, UInt32> combinedVertices)
        {
            CapnpGen.CapVertex combinedVertex = new CapnpGen.CapVertex();
            combinedVertex.Position = position;
            combinedVertex.Normal = normal;
            combinedVertex.TexCoord = texCoord;

            // If our dict of combined vertices contains a vertex identical to this already
            if (combinedVertices.ContainsKey(combinedVertex))
            {
                // Just return its index
                return combinedVertices[combinedVertex];
            }

            // Otherwise, we add the vertex to the dict and return its index
            UInt32 index = (UInt32)combinedVertices.Count;
            combinedVertices.Add(combinedVertex, index);
            return index;
        }

        ObjLoaderFactory objLoaderFactory;
        CustomMaterialStreamProvider materialStreamProvider;
        IObjLoader objLoader;
    }
}