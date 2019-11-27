using Capnp;
using Capnp.Rpc;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace CapnpGen
{
    public class CapVector3 : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            X = reader.X;
            Y = reader.Y;
            Z = reader.Z;
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            writer.X = X;
            writer.Y = Y;
            writer.Z = Z;
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public float X
        {
            get;
            set;
        }

        public float Y
        {
            get;
            set;
        }

        public float Z
        {
            get;
            set;
        }

        public struct READER
        {
            readonly DeserializerState ctx;
            public READER(DeserializerState ctx)
            {
                this.ctx = ctx;
            }

            public static READER create(DeserializerState ctx) => new READER(ctx);
            public static implicit operator DeserializerState(READER reader) => reader.ctx;
            public static implicit operator READER(DeserializerState ctx) => new READER(ctx);
            public float X => ctx.ReadDataFloat(0UL, 0F);
            public float Y => ctx.ReadDataFloat(32UL, 0F);
            public float Z => ctx.ReadDataFloat(64UL, 0F);
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(2, 0);
            }

            public float X
            {
                get => this.ReadDataFloat(0UL, 0F);
                set => this.WriteData(0UL, value, 0F);
            }

            public float Y
            {
                get => this.ReadDataFloat(32UL, 0F);
                set => this.WriteData(32UL, value, 0F);
            }

            public float Z
            {
                get => this.ReadDataFloat(64UL, 0F);
                set => this.WriteData(64UL, value, 0F);
            }
        }
    }

    public class CapVector2 : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            X = reader.X;
            Y = reader.Y;
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            writer.X = X;
            writer.Y = Y;
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public float X
        {
            get;
            set;
        }

        public float Y
        {
            get;
            set;
        }

        public struct READER
        {
            readonly DeserializerState ctx;
            public READER(DeserializerState ctx)
            {
                this.ctx = ctx;
            }

            public static READER create(DeserializerState ctx) => new READER(ctx);
            public static implicit operator DeserializerState(READER reader) => reader.ctx;
            public static implicit operator READER(DeserializerState ctx) => new READER(ctx);
            public float X => ctx.ReadDataFloat(0UL, 0F);
            public float Y => ctx.ReadDataFloat(32UL, 0F);
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(1, 0);
            }

            public float X
            {
                get => this.ReadDataFloat(0UL, 0F);
                set => this.WriteData(0UL, value, 0F);
            }

            public float Y
            {
                get => this.ReadDataFloat(32UL, 0F);
                set => this.WriteData(32UL, value, 0F);
            }
        }
    }

    public class CapVertex : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            Position = CapnpSerializable.Create<CapnpGen.CapVector3>(reader.Position);
            Normal = CapnpSerializable.Create<CapnpGen.CapVector3>(reader.Normal);
            TexCoord = CapnpSerializable.Create<CapnpGen.CapVector2>(reader.TexCoord);
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            Position?.serialize(writer.Position);
            Normal?.serialize(writer.Normal);
            TexCoord?.serialize(writer.TexCoord);
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public CapnpGen.CapVector3 Position
        {
            get;
            set;
        }

        public CapnpGen.CapVector3 Normal
        {
            get;
            set;
        }

        public CapnpGen.CapVector2 TexCoord
        {
            get;
            set;
        }

        public struct READER
        {
            readonly DeserializerState ctx;
            public READER(DeserializerState ctx)
            {
                this.ctx = ctx;
            }

            public static READER create(DeserializerState ctx) => new READER(ctx);
            public static implicit operator DeserializerState(READER reader) => reader.ctx;
            public static implicit operator READER(DeserializerState ctx) => new READER(ctx);
            public CapnpGen.CapVector3.READER Position => ctx.ReadStruct(0, CapnpGen.CapVector3.READER.create);
            public CapnpGen.CapVector3.READER Normal => ctx.ReadStruct(1, CapnpGen.CapVector3.READER.create);
            public CapnpGen.CapVector2.READER TexCoord => ctx.ReadStruct(2, CapnpGen.CapVector2.READER.create);
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(0, 3);
            }

            public CapnpGen.CapVector3.WRITER Position
            {
                get => BuildPointer<CapnpGen.CapVector3.WRITER>(0);
                set => Link(0, value);
            }

            public CapnpGen.CapVector3.WRITER Normal
            {
                get => BuildPointer<CapnpGen.CapVector3.WRITER>(1);
                set => Link(1, value);
            }

            public CapnpGen.CapVector2.WRITER TexCoord
            {
                get => BuildPointer<CapnpGen.CapVector2.WRITER>(2);
                set => Link(2, value);
            }
        }
    }

    public class CapModel : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            Vertices = reader.Vertices.ToReadOnlyList(_ => CapnpSerializable.Create<CapnpGen.CapVertex>(_));
            Indices = reader.Indices;
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            writer.Vertices.Init(Vertices, (_s1, _v1) => _v1?.serialize(_s1));
            writer.Indices.Init(Indices);
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public IReadOnlyList<CapnpGen.CapVertex> Vertices
        {
            get;
            set;
        }

        public IReadOnlyList<uint> Indices
        {
            get;
            set;
        }

        public struct READER
        {
            readonly DeserializerState ctx;
            public READER(DeserializerState ctx)
            {
                this.ctx = ctx;
            }

            public static READER create(DeserializerState ctx) => new READER(ctx);
            public static implicit operator DeserializerState(READER reader) => reader.ctx;
            public static implicit operator READER(DeserializerState ctx) => new READER(ctx);
            public IReadOnlyList<CapnpGen.CapVertex.READER> Vertices => ctx.ReadList(0).Cast(CapnpGen.CapVertex.READER.create);
            public IReadOnlyList<uint> Indices => ctx.ReadList(1).CastUInt();
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(0, 2);
            }

            public ListOfStructsSerializer<CapnpGen.CapVertex.WRITER> Vertices
            {
                get => BuildPointer<ListOfStructsSerializer<CapnpGen.CapVertex.WRITER>>(0);
                set => Link(0, value);
            }

            public ListOfPrimitivesSerializer<uint> Indices
            {
                get => BuildPointer<ListOfPrimitivesSerializer<uint>>(1);
                set => Link(1, value);
            }
        }
    }
}