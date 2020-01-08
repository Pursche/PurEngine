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

                D3D12_CPU_DESCRIPTOR_HANDLE rtv;
                D3D12_CPU_DESCRIPTOR_HANDLE srv;
                D3D12_CPU_DESCRIPTOR_HANDLE uav;
            };

            struct DepthImage
            {
                DepthImageDesc desc;
                ID3D12Resource* resource;

                ID3D12DescriptorHeap* dsvDescriptorHeap;
                ID3D12DescriptorHeap* srvDescriptorHeap;

                D3D12_CPU_DESCRIPTOR_HANDLE dsv;
                D3D12_CPU_DESCRIPTOR_HANDLE srv;
            };

        private:
            ::DXGI_FORMAT GetDXGIFormat(ImageFormat format);

            ::DXGI_FORMAT GetBaseFormat(DepthImageFormat format);
            ::DXGI_FORMAT GetDXGIFormat(DepthImageFormat format);
            ::DXGI_FORMAT GetDSVFormat(DepthImageFormat format);
            ::DXGI_FORMAT GetDepthFormat(DepthImageFormat format);
            u32 GetNumElements(DepthImageFormat format);
            
        private:
            std::vector<Image> _images;
            std::vector<DepthImage> _depthImages;
        };
    }
}