#pragma once
#include <Core.h>
#include <vector>
#include "d3dx12.h"

#include "../../../Descriptors/ImageDesc.h"
#include "../../../Descriptors/DepthImageDesc.h"

enum DXGI_FORMAT;

namespace Renderer
{
    namespace Backend
    {
        class RenderDeviceDX12;
        
        class ImageHandlerDX12
        {
        public:
            ImageHandlerDX12();
            ~ImageHandlerDX12();

            ImageID CreateImage(RenderDeviceDX12* device, const ImageDesc& desc);
            DepthImageID CreateDepthImage(RenderDeviceDX12* device, const DepthImageDesc& desc);

            const ImageDesc& GetImageDesc(const ImageID id);
            const DepthImageDesc& GetDepthImageDesc(const DepthImageID id);

        private:
            struct Image
            {
                ImageDesc desc;
                ID3D12Resource* resource;
                ID3D12DescriptorHeap* rtvDescriptorHeap;
                ID3D12DescriptorHeap* srvUavDescriptorHeap;
            };

            struct DepthImage
            {
                DepthImageDesc desc;
            };

        private:
            ::DXGI_FORMAT ImageFormatToDXGIFormat(ImageFormat format);
            ::DXGI_FORMAT DepthImageFormatToDXGIFormat(DepthImageFormat format);
            
        private:
            std::vector<Image> _images;
            std::vector<DepthImage> _depthImages;
        };
    }
}