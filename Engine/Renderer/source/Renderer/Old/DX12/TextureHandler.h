#pragma once
#include <Core.h>
#include "../TextureHandle.h"

class OldRenderDeviceDX12;

struct IWICImagingFactory;
struct ID3D12Resource;
struct D3D12_RESOURCE_DESC;

class TextureHandler
{
public:
    TextureHandler();
    TextureHandle LoadTexture(OldRenderDeviceDX12* device, const std::string& filePath);

private:
    struct LoadedTexture
    {
        std::string path;
        ID3D12Resource* textureBuffer;
        D3D12_RESOURCE_DESC* textureDesc;
        ID3D12Resource* textureBufferUploadHeap;
    };

    i32 LoadTextureFromFile(u8** imageData, D3D12_RESOURCE_DESC* resourceDescription, const std::string& filename, int& bytesPerRow);
    void CreateTexture(OldRenderDeviceDX12* device, LoadedTexture& texture, u8* imageData, int size, int bytesPerRow);
private:

    ID3D12Resource* _textureBuffer;
    IWICImagingFactory* _wicFactory;
};