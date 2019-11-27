using Capnp;
using Capnp.Rpc;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace CapnpGen
{
    public enum CapTextureType : ushort
    {
        simple
    }

    public enum CapChannel : ushort
    {
        none,
        red,
        green,
        blue,
        alpha
    }

    public class CapTextureHeader : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            Type = reader.Type;
            Width = reader.Width;
            Height = reader.Height;
            RChannel = reader.RChannel;
            GChannel = reader.GChannel;
            BChannel = reader.BChannel;
            AChannel = reader.AChannel;
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            writer.Type = Type;
            writer.Width = Width;
            writer.Height = Height;
            writer.RChannel = RChannel;
            writer.GChannel = GChannel;
            writer.BChannel = BChannel;
            writer.AChannel = AChannel;
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public CapnpGen.CapTextureType Type
        {
            get;
            set;
        }

        public uint Width
        {
            get;
            set;
        }

        public uint Height
        {
            get;
            set;
        }

        public CapnpGen.CapChannel RChannel
        {
            get;
            set;
        }

        public CapnpGen.CapChannel GChannel
        {
            get;
            set;
        }

        public CapnpGen.CapChannel BChannel
        {
            get;
            set;
        }

        public CapnpGen.CapChannel AChannel
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
            public CapnpGen.CapTextureType Type => (CapnpGen.CapTextureType)ctx.ReadDataUShort(0UL, (ushort)0);
            public uint Width => ctx.ReadDataUInt(32UL, 0U);
            public uint Height => ctx.ReadDataUInt(64UL, 0U);
            public CapnpGen.CapChannel RChannel => (CapnpGen.CapChannel)ctx.ReadDataUShort(16UL, (ushort)0);
            public CapnpGen.CapChannel GChannel => (CapnpGen.CapChannel)ctx.ReadDataUShort(96UL, (ushort)0);
            public CapnpGen.CapChannel BChannel => (CapnpGen.CapChannel)ctx.ReadDataUShort(112UL, (ushort)0);
            public CapnpGen.CapChannel AChannel => (CapnpGen.CapChannel)ctx.ReadDataUShort(128UL, (ushort)0);
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(3, 0);
            }

            public CapnpGen.CapTextureType Type
            {
                get => (CapnpGen.CapTextureType)this.ReadDataUShort(0UL, (ushort)0);
                set => this.WriteData(0UL, (ushort)value, (ushort)0);
            }

            public uint Width
            {
                get => this.ReadDataUInt(32UL, 0U);
                set => this.WriteData(32UL, value, 0U);
            }

            public uint Height
            {
                get => this.ReadDataUInt(64UL, 0U);
                set => this.WriteData(64UL, value, 0U);
            }

            public CapnpGen.CapChannel RChannel
            {
                get => (CapnpGen.CapChannel)this.ReadDataUShort(16UL, (ushort)0);
                set => this.WriteData(16UL, (ushort)value, (ushort)0);
            }

            public CapnpGen.CapChannel GChannel
            {
                get => (CapnpGen.CapChannel)this.ReadDataUShort(96UL, (ushort)0);
                set => this.WriteData(96UL, (ushort)value, (ushort)0);
            }

            public CapnpGen.CapChannel BChannel
            {
                get => (CapnpGen.CapChannel)this.ReadDataUShort(112UL, (ushort)0);
                set => this.WriteData(112UL, (ushort)value, (ushort)0);
            }

            public CapnpGen.CapChannel AChannel
            {
                get => (CapnpGen.CapChannel)this.ReadDataUShort(128UL, (ushort)0);
                set => this.WriteData(128UL, (ushort)value, (ushort)0);
            }
        }
    }

    public class CapTexture : ICapnpSerializable
    {
        void ICapnpSerializable.Deserialize(DeserializerState arg_)
        {
            var reader = READER.create(arg_);
            Header = CapnpSerializable.Create<CapnpGen.CapTextureHeader>(reader.Header);
            Data = reader.Data;
            applyDefaults();
        }

        public void serialize(WRITER writer)
        {
            Header?.serialize(writer.Header);
            writer.Data.Init(Data);
        }

        void ICapnpSerializable.Serialize(SerializerState arg_)
        {
            serialize(arg_.Rewrap<WRITER>());
        }

        public void applyDefaults()
        {
        }

        public CapnpGen.CapTextureHeader Header
        {
            get;
            set;
        }

        public IReadOnlyList<byte> Data
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
            public CapnpGen.CapTextureHeader.READER Header => ctx.ReadStruct(0, CapnpGen.CapTextureHeader.READER.create);
            public IReadOnlyList<byte> Data => ctx.ReadList(1).CastByte();
        }

        public class WRITER : SerializerState
        {
            public WRITER()
            {
                this.SetStruct(0, 2);
            }

            public CapnpGen.CapTextureHeader.WRITER Header
            {
                get => BuildPointer<CapnpGen.CapTextureHeader.WRITER>(0);
                set => Link(0, value);
            }

            public ListOfPrimitivesSerializer<byte> Data
            {
                get => BuildPointer<ListOfPrimitivesSerializer<byte>>(1);
                set => Link(1, value);
            }
        }
    }
}