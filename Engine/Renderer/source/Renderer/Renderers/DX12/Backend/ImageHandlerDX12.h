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

            const ImageDesc& GetDescriptor(const ImageID id);
            const DepthImageDesc& GetDescriptor(const DepthImageID id);

            DXGI_FORMAT GetDXGIFormat(const ImageID id);
            DXGI_FORMAT GetDXGIFormat(const DepthImageID id);

            ID3D12Resource* GetResource(const ImageID id);
            ID3D12Resource* GetResource(const DepthImageID id);

            D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(const ImageID id);
            D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(const ImageID id);
            ID3D12DescriptorHeap* GetSRVDescriptorHeap(const ImageID id);
            D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(const ImageID id);

            D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(const DepthImageID id);
            D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(const DepthImageID id);
            ID3D12DescriptorHeap* GetSRVDescriptorHeap(const DepthImageID id);

        private:
            struct Image
            {
                ImageDesc desc;
                Microsoft::WRL::ComPtr<ID3D12Resource> resource;

                Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
                Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvUavDescriptorHeap;

                D3D12_CPU_DESCRIPTOR_HANDLE rtv;
                D3D12_CPU_DESCRIPTOR_HANDLE srv;
                D3D12_CPU_DESCRIPTOR_HANDLE uav;
            };

            struct DepthImage
            {
                DepthImageDesc desc;
                Microsoft::WRL::ComPtr<ID3D12Resource> resource;

                Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
                Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

                D3D12_CPU_DESCRIPTOR_HANDLE dsv;
                D3D12_CPU_DESCRIPTOR_HANDLE srv;
            };

        private:
            static ::DXGI_FORMAT ToDXGIFormat(ImageFormat format);

            static ::DXGI_FORMAT ToBaseFormat(DepthImageFormat format);
            static ::DXGI_FORMAT ToDXGIFormat(DepthImageFormat format);
            static ::DXGI_FORMAT ToDSVFormat(DepthImageFormat format);
            static ::DXGI_FORMAT ToDepthFormat(DepthImageFormat format);
            static u32 ToNumElements(DepthImageFormat format);
            
        private:
            std::vector<Image> _images;
            std::vector<DepthImage> _depthImages;
        };
    }
}