using Capnp;
using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

namespace Cooker.Cookers
{
    class BmpTextureCooker : BaseCooker
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
            string assetPath = GetAssetPath(luaPath);

            if (!assetPath.EndsWith(".bmp"))
            {
                error = "We only convert .bmp files";
                return false;
            }

            return File.Exists(assetPath);
        }

        public override bool Cook(string luaPath, LuaGlobal environment, string outputDirectory, string intermediateDirectory, out string producedFile, out string error)
        {
            error = "";
            producedFile = "";
            string assetPath = GetAssetPath(luaPath);
            using (FileStream assetStream = new FileStream(assetPath, FileMode.Open))
            {
                using (Bitmap bmp = new Bitmap(Image.FromStream(assetStream)))
                {
                    string channelMask = Env.TryGetDefault<string>(environment, "channelMask", "RGBA");

                    // Try to get all settings from .lua file or acceptable defaults
                    CapnpGen.CapTexture texture = new CapnpGen.CapTexture();

                    texture.Header = new CapnpGen.CapTextureHeader();
                    texture.Header.Type = CapnpGen.CapTextureType.simple; // TODO: DDS support?

                    texture.Header.Width = (uint)bmp.Width;
                    texture.Header.Height = (uint)bmp.Height;

                    texture.Header.RChannel = GetChannel(channelMask, 0);
                    texture.Header.GChannel = GetChannel(channelMask, 1);
                    texture.Header.BChannel = GetChannel(channelMask, 2);
                    texture.Header.AChannel = GetChannel(channelMask, 3);

                    PixelFormat pixelFormat = bmp.PixelFormat;
                    int bytesPerPixel = GetBytesPerPixel(pixelFormat);

                    // Lock the bitmap's bits.  
                    Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
                    BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.ReadWrite, bmp.PixelFormat);

                    // Get the address of the first line.
                    IntPtr ptr = bmpData.Scan0;

                    // Declare an array to hold the bytes of the bitmap.
                    int bytes = bmp.Width * bmp.Height * bytesPerPixel;
                    byte[] byteValues = new byte[bytes];

                    // Copy the color values into the array.
                    Marshal.Copy(ptr, byteValues, 0, bytes);

                    MemoryStream memoryStream = new MemoryStream();

                    // Loop over the pixels
                    int stride = bmpData.Width;
                    for (int row = 0; row < bmpData.Height; row++)
                    {
                        for (int column = 0; column < bmpData.Width; column++)
                        {
                            int pixelIndex = ((row * stride) + column) * bytesPerPixel;

                            WriteChannel(texture.Header.RChannel, byteValues, pixelIndex, pixelFormat, memoryStream);
                            WriteChannel(texture.Header.GChannel, byteValues, pixelIndex, pixelFormat, memoryStream);
                            WriteChannel(texture.Header.BChannel, byteValues, pixelIndex, pixelFormat, memoryStream);
                            WriteChannel(texture.Header.AChannel, byteValues, pixelIndex, pixelFormat, memoryStream);
                        }
                    }

                    texture.Data = memoryStream.ToArray();

                    bmp.UnlockBits(bmpData);

                    // Create output file
                    producedFile = Path.ChangeExtension(Path.GetFileName(assetPath), ".PurTexture");
                    string outFilePath = Path.Combine(outputDirectory, "textures", producedFile);
                    Directory.CreateDirectory(Path.GetDirectoryName(outFilePath));
                    using (FileStream outStream = new FileStream(outFilePath, FileMode.Create))
                    {
                        MessageBuilder message = MessageBuilder.Create();
                        var root = message.BuildRoot<CapnpGen.CapTexture.WRITER>();
                        texture.serialize(root);

                        var pump = new FramePump(outStream);
                        pump.Send(message.Frame);
                    }
                }

            }


            return true;
        }

        CapnpGen.CapChannel GetChannel(string channelMask, int channel)
        {
            char channelChar = channelMask[channel];

            switch (channelChar)
            {
                case 'N': return CapnpGen.CapChannel.none;
                case 'R': return CapnpGen.CapChannel.red;
                case 'G': return CapnpGen.CapChannel.green;
                case 'B': return CapnpGen.CapChannel.blue;
                case 'A': return CapnpGen.CapChannel.alpha;
            }

            Debug.Fail("Unrecognized channel!");
            return CapnpGen.CapChannel.none;
        }

        int GetBytesPerPixel(PixelFormat pixelFormat)
        {
            switch (pixelFormat)
            {
                case PixelFormat.Format64bppArgb:
                case PixelFormat.Format64bppPArgb:
                    return 8;

                case PixelFormat.Format32bppArgb:
                case PixelFormat.Format32bppRgb:
                case PixelFormat.Format32bppPArgb:
                    return 4;

                case PixelFormat.Format24bppRgb:
                    return 3;

                case PixelFormat.Format16bppArgb1555:
                case PixelFormat.Format16bppGrayScale:
                case PixelFormat.Format16bppRgb555:
                case PixelFormat.Format16bppRgb565:
                    return 2;

                default:
                    Debug.Fail("Unsupported texture format!");
                    return -1;
            }
        }

        void WriteChannel(CapnpGen.CapChannel channel, byte[] pixels, int pixelByteIndex, PixelFormat pixelFormat, MemoryStream memory)
        {
            switch (channel)
            {
                case CapnpGen.CapChannel.none:
                    return;
                case CapnpGen.CapChannel.red:
                    WriteRed(pixels, pixelByteIndex, pixelFormat, memory);
                    return;
                case CapnpGen.CapChannel.green:
                    WriteGreen(pixels, pixelByteIndex, pixelFormat, memory);
                    return;
                case CapnpGen.CapChannel.blue:
                    WriteBlue(pixels, pixelByteIndex, pixelFormat, memory);
                    return;
                case CapnpGen.CapChannel.alpha:
                    WriteAlpha(pixels, pixelByteIndex, pixelFormat, memory);
                    return;
            }
        }

        void WriteRed(byte[] pixels, int pixelByteIndex, PixelFormat pixelFormat, MemoryStream memory)
        {
            int byteSize = 0;
            bool isPackedFormat = false;

            switch (pixelFormat)
            {
                case PixelFormat.Format64bppArgb:
                case PixelFormat.Format64bppPArgb:
                    pixelByteIndex += 1 * 2;
                    byteSize = 2;
                    break;

                case PixelFormat.Format32bppArgb:
                case PixelFormat.Format32bppPArgb:
                    pixelByteIndex += 1 * 1;
                    byteSize = 1;
                    break;
                case PixelFormat.Format24bppRgb:
                case PixelFormat.Format32bppRgb:
                    pixelByteIndex += 1 * 0;
                    byteSize = 1;
                    break;

                case PixelFormat.Format16bppGrayScale: // Greyscale, just return the grey color
                    pixelByteIndex += 1 * 0;
                    byteSize = 2;
                    break;

                case PixelFormat.Format16bppArgb1555:
                case PixelFormat.Format16bppRgb555:
                case PixelFormat.Format16bppRgb565:
                    isPackedFormat = true; // figure it out later
                    break;

                default:
                    Debug.Fail("Unsupported texture format!");
                    return;
            }

            if (byteSize == 1)
            {
                byte color = pixels[pixelByteIndex];
                memory.WriteByte(color);
            }
            else if (byteSize == 2 && !isPackedFormat)
            {
                byte[] color = pixels.SubArray(pixelByteIndex, byteSize);
                foreach (byte colorByte in color)
                {
                    memory.WriteByte(colorByte);
                }
            }
            else if (isPackedFormat)
            {
                if (pixelFormat == PixelFormat.Format16bppArgb1555) // Grey scale, just read
                {
                    byte color = pixels[pixelByteIndex]; // Color looks like this: ARRRRRGG
                    color = (byte)(color << 1); // Color will now look like this: RRRRRGG0
                    color = (byte)(color >> 3); // Color will now look like this: 000RRRRR, and this is what we want
                    memory.WriteByte(color);
                }
                else if (pixelFormat == PixelFormat.Format16bppRgb555 || pixelFormat == PixelFormat.Format16bppRgb565)
                {
                    byte color = pixels[pixelByteIndex]; // Color looks like this: RRRRRGGG
                    color = (byte)(color >> 3); // Color will now look like this: 000RRRRR, and this is what we want
                    memory.WriteByte(color);
                }
            }
        }
        void WriteGreen(byte[] pixels, int pixelByteIndex, PixelFormat pixelFormat, MemoryStream memory)
        {
            int byteSize = 0;
            bool isPackedFormat = false;

            switch (pixelFormat)
            {
                case PixelFormat.Format64bppArgb:
                case PixelFormat.Format64bppPArgb:
                    pixelByteIndex += 4;
                    byteSize = 2;
                    break;

                case PixelFormat.Format32bppArgb:
                case PixelFormat.Format32bppPArgb:
                    pixelByteIndex += 2;
                    byteSize = 1;
                    break;
                case PixelFormat.Format24bppRgb:
                case PixelFormat.Format32bppRgb:
                    pixelByteIndex += 1;
                    byteSize = 1;
                    break;

                case PixelFormat.Format16bppGrayScale: // Greyscale, just return the grey color
                    pixelByteIndex += 0;
                    byteSize = 2;
                    break;

                case PixelFormat.Format16bppArgb1555:
                case PixelFormat.Format16bppRgb555:
                case PixelFormat.Format16bppRgb565:
                    isPackedFormat = true; // figure it out later
                    break;

                default:
                    Debug.Fail("Unsupported texture format!");
                    return;
            }

            if (byteSize == 1)
            {
                byte color = pixels[pixelByteIndex];
                memory.WriteByte(color);
            }
            else if (byteSize == 2 && !isPackedFormat)
            {
                byte[] color = pixels.SubArray(pixelByteIndex, byteSize);
                foreach (byte colorByte in color)
                {
                    memory.WriteByte(colorByte);
                }
            }
            else if (isPackedFormat)
            {
                if (pixelFormat == PixelFormat.Format16bppArgb1555) // UNTESTED
                {
                    byte colorHigh = pixels[pixelByteIndex]; // ColorHigh looks like this: ARRRRRGG
                    colorHigh = (byte)(colorHigh << 6); // ColorHigh will now look like this: GG000000
                    colorHigh = (byte)(colorHigh >> 3); // ColorHigh will now look like this: 000GG000

                    byte colorLow = pixels[pixelByteIndex + 1]; // ColorLow looks like this: GGGBBBBB
                    colorLow = (byte)(colorLow >> 5); // ColorLow will now look like this: 00000GGG

                    byte color = (byte)(colorHigh | colorLow); // Color will now look like this: 000GGGGG, and this is what we want
                    memory.WriteByte(color);
                }
                else if (pixelFormat == PixelFormat.Format16bppRgb555)// UNTESTED
                {
                    byte colorHigh = pixels[pixelByteIndex]; // ColorHigh looks like this: RRRRRGGG
                    colorHigh = (byte)(colorHigh << 5); // ColorHigh will now look like this: GGG00000
                    colorHigh = (byte)(colorHigh >> 3); // ColorHigh will now look like this: 000GGG00

                    byte colorLow = pixels[pixelByteIndex + 1]; // ColorLow looks like this: GGBBBBB0
                    colorLow = (byte)(colorLow >> 6); // ColorLow will now look like this: 000000GG

                    byte color = (byte)(colorHigh | colorLow); // Color will now look like this: 000GGGGG, and this is what we want
                    memory.WriteByte(color);
                }
                else if (pixelFormat == PixelFormat.Format16bppRgb565)// UNTESTED
                {
                    byte colorHigh = pixels[pixelByteIndex]; // ColorHigh looks like this: RRRRRGGG
                    colorHigh = (byte)(colorHigh << 5); // ColorHigh will now look like this: GGG00000
                    colorHigh = (byte)(colorHigh >> 2); // ColorHigh will now look like this: 00GGG000

                    byte colorLow = pixels[pixelByteIndex + 1]; // ColorLow looks like this: GGGBBBBB
                    colorLow = (byte)(colorLow >> 5); // ColorLow will now look like this: 00000GGG

                    byte color = (byte)(colorHigh | colorLow); // Color will now look like this: 00GGGGGG, and this is what we want
                    memory.WriteByte(color);
                }
            }
        }
        void WriteBlue(byte[] pixels, int pixelByteIndex, PixelFormat pixelFormat, MemoryStream memory)
        {
            int byteSize = 0;
            bool isPackedFormat = false;

            switch (pixelFormat)
            {
                case PixelFormat.Format64bppArgb:
                case PixelFormat.Format64bppPArgb:
                    pixelByteIndex += 6;
                    byteSize = 2;
                    break;

                case PixelFormat.Format32bppArgb:
                case PixelFormat.Format32bppPArgb:
                    pixelByteIndex += 3;
                    byteSize = 1;
                    break;
                case PixelFormat.Format24bppRgb:
                case PixelFormat.Format32bppRgb:
                    pixelByteIndex += 2;
                    byteSize = 1;
                    break;

                case PixelFormat.Format16bppGrayScale: // Greyscale, just return the grey color
                    pixelByteIndex += 0;
                    byteSize = 2;
                    break;

                case PixelFormat.Format16bppArgb1555:
                case PixelFormat.Format16bppRgb555:
                case PixelFormat.Format16bppRgb565:
                    isPackedFormat = true; // figure it out later
                    break;

                default:
                    Debug.Fail("Unsupported texture format!");
                    return;
            }

            if (byteSize == 1)
            {
                byte color = pixels[pixelByteIndex];
                memory.WriteByte(color);
            }
            else if (byteSize == 2 && !isPackedFormat)
            {
                byte[] color = pixels.SubArray(pixelByteIndex, byteSize);
                foreach (byte colorByte in color)
                {
                    memory.WriteByte(colorByte);
                }
            }
            else if (isPackedFormat)
            {
                if (pixelFormat == PixelFormat.Format16bppArgb1555 || pixelFormat == PixelFormat.Format16bppRgb565) // UNTESTED
                {
                    byte color = pixels[pixelByteIndex + 1]; // Color looks like this: GGGBBBBB
                    color = (byte)(color << 3); // Color will now look like this: BBBBB000
                    color = (byte)(color >> 3);// Color will now look like this: 000BBBBB, and this is what we want

                    memory.WriteByte(color);
                }
                else if (pixelFormat == PixelFormat.Format16bppRgb555)
                {
                    byte color = pixels[pixelByteIndex + 1]; // Color looks like this: GGBBBBB0
                    color = (byte)(color << 2); // Color will now look like this: BBBBB000
                    color = (byte)(color >> 3);// Color will now look like this: 000BBBBB, and this is what we want

                    memory.WriteByte(color);
                }
            }
        }
        void WriteAlpha(byte[] pixels, int pixelByteIndex, PixelFormat pixelFormat, MemoryStream memory)
        {
            int byteSize = 0;
            bool isPackedFormat = false;

            switch (pixelFormat)
            {
                case PixelFormat.Format64bppArgb:
                case PixelFormat.Format64bppPArgb:
                    pixelByteIndex += 0;
                    byteSize = 2;
                    break;

                case PixelFormat.Format32bppArgb:
                case PixelFormat.Format32bppPArgb:
                    pixelByteIndex += 0;
                    byteSize = 1;
                    break;

                case PixelFormat.Format16bppArgb1555:
                    isPackedFormat = true; // figure it out later
                    break;

                // These don't have alpha, don't write anything
                case PixelFormat.Format24bppRgb:
                case PixelFormat.Format32bppRgb:
                case PixelFormat.Format16bppGrayScale:
                case PixelFormat.Format16bppRgb555:
                case PixelFormat.Format16bppRgb565:
                    return;

                default:
                    Debug.Fail("Unsupported texture format!");
                    return;
            }

            if (byteSize == 1)
            {
                byte color = pixels[pixelByteIndex];
                memory.WriteByte(color);
            }
            else if (byteSize == 2 && !isPackedFormat)
            {
                byte[] color = pixels.SubArray(pixelByteIndex, byteSize);
                foreach (byte colorByte in color)
                {
                    memory.WriteByte(colorByte);
                }
            }
            else if (isPackedFormat)
            {
                if (pixelFormat == PixelFormat.Format16bppArgb1555) // UNTESTED
                {
                    byte color = pixels[pixelByteIndex]; // Color looks like this: ARRRRRGG
                    color = (byte)(color >> 7);// Color will now look like this: 0000000A, and this is what we want

                    memory.WriteByte(color);
                }

            }
        }
    }
}