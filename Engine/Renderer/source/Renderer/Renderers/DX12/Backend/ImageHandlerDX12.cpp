#include "ImageHandlerDX12.h"
#include "RenderDeviceDX12.h"
#include <cassert>

namespace Renderer
{
    namespace Backend
    {
        ImageHandlerDX12::ImageHandlerDX12()
        {

        }

        ImageHandlerDX12::~ImageHandlerDX12()
        {

        }

        ImageID ImageHandlerDX12::CreateImage(RenderDeviceDX12* device, const ImageDesc& desc)
        {
            size_t nextHandle = _images.size();

            // Make sure we haven't exceeded the limit of the ImageID type, if this hits you need to change type of ImageID to something bigger
            assert(nextHandle < ImageID::MaxValue());
            using type = type_safe::underlying_type<ImageID>;

            Image image;
            image.desc = desc;

            // Create resource
            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            D3D12_RESOURCE_DESC resourceDesc;

            assert(desc.dimensions.x > 0); // Make sure the width is valid
            assert(desc.dimensions.y > 0); // Make sure the height is valid
            assert(desc.depth > 0); // Make sure the depth is valid
            assert(desc.format != IMAGE_FORMAT_UNKNOWN); // Make sure the format is valid

            DXGI_FORMAT format = GetDXGIFormat(desc.format);
            int sampleQuality = (desc.sampleCount == SAMPLE_COUNT_1) ? 0 : SampleCountToInt(desc.sampleCount);

            if (desc.depth == 1) // 2d image
            {
                resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format,
                    desc.dimensions.x, desc.dimensions.y,
                    1, 1, SampleCountToInt(desc.sampleCount), sampleQuality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            }
            else
            {
                assert(false); // Non-2d images is currently unsupported
            }

            D3D12_CLEAR_VALUE clearValue = { format, { desc.clearColor.x, desc.clearColor.y, desc.clearColor.z, desc.clearColor.w } };

            HRESULT result = device->_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&image.resource));
            assert(SUCCEEDED(result)); // Failed to create commited resource

            // Create RTV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            { 
                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
                rtvHeapDesc.NumDescriptors = 1;
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                result = device->_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&image.rtvDescriptorHeap));
                assert(SUCCEEDED(result)); // Failed to create RTV descriptor heap
            }

            // Create RTV
            image.rtv = image.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

            device->_device->CreateRenderTargetView(image.resource, nullptr, image.rtv);

            // Create SRV/UAV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};
                srvUavHeapDesc.NumDescriptors = 2;
                srvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                srvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                result = device->_device->CreateDescriptorHeap(&srvUavHeapDesc, IID_PPV_ARGS(&image.srvUavDescriptorHeap));
                assert(SUCCEEDED(result)); // Failed to create SRV/UAV descriptor heap
            }
            
            auto srvUavDescriptorSize = device->_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            // Create SRV
            image.srv = image.srvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

            device->_device->CreateShaderResourceView(image.resource, nullptr, image.srv); // TODO: Unsure if this works, do we need a descriptor as well?

            // Create UAV
            image.uav = image.srvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            image.uav.ptr += srvUavDescriptorSize;
            
            device->_device->CreateUnorderedAccessView(image.resource, nullptr, nullptr, image.uav); // TODO: Unsure if this works, do we need a descriptor as well? What about counter resource?

            _images.push_back(image);

            return ImageID(static_cast<type>(nextHandle));
        }

        DepthImageID ImageHandlerDX12::CreateDepthImage(RenderDeviceDX12* device, const DepthImageDesc& desc)
        {
            size_t nextHandle = _images.size();

            // Make sure we haven't exceeded the limit of the ImageID type, if this hits you need to change type of ImageID to something bigger
            assert(nextHandle < DepthImageID::MaxValue());
            using type = type_safe::underlying_type<DepthImageID>;

            DepthImage image;
            image.desc = desc;

            // Create resource
            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            D3D12_RESOURCE_DESC resourceDesc;

            assert(desc.dimensions.x > 0); // Make sure the width is valid
            assert(desc.dimensions.y > 0); // Make sure the height is valid
            assert(desc.format != IMAGE_FORMAT_UNKNOWN); // Make sure the format is valid

            int sampleQuality = (desc.sampleCount == SAMPLE_COUNT_1) ? 0 : SampleCountToInt(desc.sampleCount);

            resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(GetBaseFormat(desc.format),
                desc.dimensions.x, desc.dimensions.y,
                1, 1, SampleCountToInt(desc.sampleCount), sampleQuality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = GetDXGIFormat(desc.format);
            clearValue.DepthStencil.Depth = desc.depthClearValue;
            clearValue.DepthStencil.Stencil = desc.stencilClearValue;

            HRESULT result = device->_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&image.resource));
            assert(SUCCEEDED(result)); // Failed to create commited resource

            // Create DSV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
                dsvHeapDesc.NumDescriptors = 1;
                dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                result = device->_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&image.dsvDescriptorHeap));
                assert(SUCCEEDED(result)); // Failed to create DSV descriptor heap
            }

            // Create DSV
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = GetDSVFormat(desc.format);

            if (desc.sampleCount == SAMPLE_COUNT_1)
            {
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = 0;
            }
            else
            {
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
            }

            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

            image.dsv = image.dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            device->_device->CreateDepthStencilView(image.resource, &dsvDesc, image.dsv);

            // Create SRV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
                srvHeapDesc.NumDescriptors = 1;
                srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                result = device->_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&image.srvDescriptorHeap));
                assert(SUCCEEDED(result)); // Failed to create SRV descriptor heap
            }

            // Create SRV
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = GetDepthFormat(desc.format);

            if (dsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            image.srv = image.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            device->_device->CreateShaderResourceView(image.resource, &srvDesc, image.srv); // TODO: Unsure if this works, do we need a descriptor as well?

            _depthImages.push_back(image);

            return DepthImageID(static_cast<type>(nextHandle));
        }

        const ImageDesc& ImageHandlerDX12::GetImageDesc(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));
            return _images[static_cast<type>(id)].desc;
        }

        const DepthImageDesc& ImageHandlerDX12::GetDepthImageDesc(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].desc;
        }

        DXGI_FORMAT ImageHandlerDX12::GetDXGIFormat(ImageFormat format)
        {
            switch (format)
            {
                case IMAGE_FORMAT_UNKNOWN: assert(false); break; // This is an invalid format
                case IMAGE_FORMAT_R32G32B32A32_TYPELESS:    return DXGI_FORMAT_R32G32B32A32_TYPELESS; // RGBA32, 128 bits per pixel
                case IMAGE_FORMAT_R32G32B32A32_FLOAT:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case IMAGE_FORMAT_R32G32B32A32_UINT:        return DXGI_FORMAT_R32G32B32A32_UINT;
                case IMAGE_FORMAT_R32G32B32A32_SINT:        return DXGI_FORMAT_R32G32B32A32_SINT;
                case IMAGE_FORMAT_R32G32B32_TYPELESS:       return DXGI_FORMAT_R32G32B32_TYPELESS; // RGB32, 96 bits per pixel
                case IMAGE_FORMAT_R32G32B32_FLOAT:          return DXGI_FORMAT_R32G32B32_FLOAT;
                case IMAGE_FORMAT_R32G32B32_UINT:           return DXGI_FORMAT_R32G32B32_UINT;
                case IMAGE_FORMAT_R32G32B32_SINT:           return DXGI_FORMAT_R32G32B32_SINT;
                case IMAGE_FORMAT_R16G16B16A16_TYPELESS:    return DXGI_FORMAT_R16G16B16A16_TYPELESS; // RGBA16, 64 bits per pixel
                case IMAGE_FORMAT_R16G16B16A16_FLOAT:       return DXGI_FORMAT_R16G16B16A16_FLOAT;
                case IMAGE_FORMAT_R16G16B16A16_UNORM:       return DXGI_FORMAT_R16G16B16A16_UNORM;
                case IMAGE_FORMAT_R16G16B16A16_UINT:        return DXGI_FORMAT_R16G16B16A16_UINT;
                case IMAGE_FORMAT_R16G16B16A16_SNORM:       return DXGI_FORMAT_R16G16B16A16_SNORM;
                case IMAGE_FORMAT_R16G16B16A16_SINT:        return DXGI_FORMAT_R16G16B16A16_SINT;
                case IMAGE_FORMAT_R32G32_TYPELESS:          return DXGI_FORMAT_R32G32_TYPELESS; // RG32, 64 bits per pixel
                case IMAGE_FORMAT_R32G32_FLOAT:             return DXGI_FORMAT_R32G32_FLOAT;
                case IMAGE_FORMAT_R32G32_UINT:              return DXGI_FORMAT_R32G32_UINT;
                case IMAGE_FORMAT_R32G32_SINT:              return DXGI_FORMAT_R32G32_SINT;
                case IMAGE_FORMAT_R10G10B10A2_TYPELESS:     return DXGI_FORMAT_R10G10B10A2_TYPELESS; // RGB10A2, 32 bits per pixel
                case IMAGE_FORMAT_R10G10B10A2_UNORM:        return DXGI_FORMAT_R10G10B10A2_UNORM;
                case IMAGE_FORMAT_R10G10B10A2_UINT:         return DXGI_FORMAT_R10G10B10A2_UINT;
                case IMAGE_FORMAT_R11G11B10_FLOAT:          return DXGI_FORMAT_R11G11B10_FLOAT; //RG11B10, 32 bits per pixel
                case IMAGE_FORMAT_R8G8B8A8_TYPELESS:        return DXGI_FORMAT_R8G8B8A8_TYPELESS; //RGBA8, 32 bits per pixel
                case IMAGE_FORMAT_R8G8B8A8_UNORM:           return DXGI_FORMAT_R8G8B8A8_UNORM;
                case IMAGE_FORMAT_R8G8B8A8_UNORM_SRGB:      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case IMAGE_FORMAT_R8G8B8A8_UINT:            return DXGI_FORMAT_R8G8B8A8_UINT;
                case IMAGE_FORMAT_R8G8B8A8_SNORM:           return DXGI_FORMAT_R8G8B8A8_SNORM;
                case IMAGE_FORMAT_R8G8B8A8_SINT:            return DXGI_FORMAT_R8G8B8A8_SINT;
                case IMAGE_FORMAT_R16G16_TYPELESS:          return DXGI_FORMAT_R16G16_TYPELESS; // RG16, 32 bits per pixel
                case IMAGE_FORMAT_R16G16_FLOAT:             return DXGI_FORMAT_R16G16_FLOAT;
                case IMAGE_FORMAT_R16G16_UNORM:             return DXGI_FORMAT_R16G16_UNORM;
                case IMAGE_FORMAT_R16G16_UINT:              return DXGI_FORMAT_R16G16_UINT;
                case IMAGE_FORMAT_R16G16_SNORM:             return DXGI_FORMAT_R16G16_SNORM;
                case IMAGE_FORMAT_R16G16_SINT:              return DXGI_FORMAT_R16G16_SINT;
                case IMAGE_FORMAT_R32_TYPELESS:             return DXGI_FORMAT_R32_TYPELESS; // R32, 32 bits per pixel
                case IMAGE_FORMAT_R32_FLOAT:                return DXGI_FORMAT_R32_FLOAT;
                case IMAGE_FORMAT_R32_UINT:                 return DXGI_FORMAT_R32_UINT;
                case IMAGE_FORMAT_R32_SINT:                 return DXGI_FORMAT_R32_SINT;
                case IMAGE_FORMAT_R24G8_TYPELESS:           return DXGI_FORMAT_R24G8_TYPELESS; // R24G8, 32 bits per pixel
                case IMAGE_FORMAT_R8G8_TYPELESS:            return DXGI_FORMAT_R8G8_TYPELESS; // RG8, 16 bits per pixel
                case IMAGE_FORMAT_R8G8_UNORM:               return DXGI_FORMAT_R8G8_UNORM;
                case IMAGE_FORMAT_R8G8_UINT:                return DXGI_FORMAT_R8G8_UINT;
                case IMAGE_FORMAT_R8G8_SNORM:               return DXGI_FORMAT_R8G8_SNORM;
                case IMAGE_FORMAT_R8G8_SINT:                return DXGI_FORMAT_R8G8_SINT;
                case IMAGE_FORMAT_R16_TYPELESS:             return DXGI_FORMAT_R16_TYPELESS; // R16, 16 bits per pixel
                case IMAGE_FORMAT_R16_FLOAT:                return DXGI_FORMAT_R16_FLOAT;
                case IMAGE_FORMAT_D16_UNORM:                return DXGI_FORMAT_D16_UNORM; // Depth instead of Red
                case IMAGE_FORMAT_R16_UNORM:                return DXGI_FORMAT_R16_UNORM;
                case IMAGE_FORMAT_R16_UINT:                 return DXGI_FORMAT_R16_UINT;
                case IMAGE_FORMAT_R16_SNORM:                return DXGI_FORMAT_R16_SNORM;
                case IMAGE_FORMAT_R16_SINT:                 return DXGI_FORMAT_R16_SINT;
                case IMAGE_FORMAT_R8_TYPELESS:              return DXGI_FORMAT_R8_TYPELESS; // R8, 8 bits per pixel
                case IMAGE_FORMAT_R8_UNORM:                 return DXGI_FORMAT_R8_UNORM;
                case IMAGE_FORMAT_R8_UINT:                  return DXGI_FORMAT_R8_UINT;
                case IMAGE_FORMAT_R8_SNORM:                 return DXGI_FORMAT_R8_SNORM;
                case IMAGE_FORMAT_R8_SINT:                  return DXGI_FORMAT_R8_SINT;
                case IMAGE_FORMAT_A8_UNORM:                 return DXGI_FORMAT_A8_UNORM; // A8, 8 bits per pixel
                case IMAGE_FORMAT_R1_UNORM:                 return DXGI_FORMAT_R1_UNORM; // R1, 1 bit per pixel
                default:
                    assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ImageHandlerDX12::GetBaseFormat(DepthImageFormat format)
        {
            switch(format)
            {
                case DEPTH_IMAGE_FORMAT_UNKNOWN:
                    assert(false); // This is an invalid format

                // 32-bit Z w/ Stencil
                case DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS:
                case DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT:
                case DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS:
                case DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                // No Stencil
                case DEPTH_IMAGE_FORMAT_R32_TYPELESS:
                case DEPTH_IMAGE_FORMAT_D32_FLOAT:
                case DEPTH_IMAGE_FORMAT_R32_FLOAT:
                    return DXGI_FORMAT_R32_TYPELESS;

                // 24-bit Z
                case DEPTH_IMAGE_FORMAT_R24G8_TYPELESS:
                case DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT:
                case DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS:
                case DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                
                // 16-bit Z w/o Stencil
                case DEPTH_IMAGE_FORMAT_R16_TYPELESS:
                case DEPTH_IMAGE_FORMAT_D16_UNORM:
                case DEPTH_IMAGE_FORMAT_R16_UNORM:
                    return DXGI_FORMAT_R16_TYPELESS;

                default:
                    assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ImageHandlerDX12::GetDXGIFormat(DepthImageFormat format)
        {
            switch (format)
            {
                case DEPTH_IMAGE_FORMAT_UNKNOWN:
                assert(false); // This is an invalid format

                // 32-bit Z w/ Stencil
                case DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS:          return DXGI_FORMAT_R32G8X24_TYPELESS;
                case DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT:       return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                case DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS:   return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                case DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT:    return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

                    // No Stencil
                case DEPTH_IMAGE_FORMAT_R32_TYPELESS:               return DXGI_FORMAT_R32_TYPELESS;
                case DEPTH_IMAGE_FORMAT_D32_FLOAT:                  return DXGI_FORMAT_D32_FLOAT;
                case DEPTH_IMAGE_FORMAT_R32_FLOAT:                  return DXGI_FORMAT_R32_FLOAT;

                    // 24-bit Z
                case DEPTH_IMAGE_FORMAT_R24G8_TYPELESS:             return DXGI_FORMAT_R24G8_TYPELESS;
                case DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT:          return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                case DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT:       return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

                    // 16-bit Z w/o Stencil
                case DEPTH_IMAGE_FORMAT_R16_TYPELESS:               return DXGI_FORMAT_R16_TYPELESS;
                case DEPTH_IMAGE_FORMAT_D16_UNORM:                  return DXGI_FORMAT_D16_UNORM;
                case DEPTH_IMAGE_FORMAT_R16_UNORM:                  return DXGI_FORMAT_R16_UNORM;

                default:
                    assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ImageHandlerDX12::GetDSVFormat(DepthImageFormat format)
        {
            switch (format)
            {
            case DEPTH_IMAGE_FORMAT_UNKNOWN:
                assert(false); // This is an invalid format

            // 32-bit Z w/ Stencil
            case DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT:
            case DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

                // No Stencil
            case DEPTH_IMAGE_FORMAT_R32_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D32_FLOAT:
            case DEPTH_IMAGE_FORMAT_R32_FLOAT:
                return DXGI_FORMAT_D32_FLOAT;

                // 24-bit Z
            case DEPTH_IMAGE_FORMAT_R24G8_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT:
            case DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS:
            case DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;

                // 16-bit Z w/o Stencil
            case DEPTH_IMAGE_FORMAT_R16_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D16_UNORM:
            case DEPTH_IMAGE_FORMAT_R16_UNORM:
                return DXGI_FORMAT_D16_UNORM;

            default:
                assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        DXGI_FORMAT ImageHandlerDX12::GetDepthFormat(DepthImageFormat format)
        {
            switch (format)
            {
            case DEPTH_IMAGE_FORMAT_UNKNOWN:
                assert(false); // This is an invalid format

            // 32-bit Z w/ Stencil
            case DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT:
            case DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT:
                return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

                // No Stencil
            case DEPTH_IMAGE_FORMAT_R32_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D32_FLOAT:
            case DEPTH_IMAGE_FORMAT_R32_FLOAT:
                return DXGI_FORMAT_R32_FLOAT;

                // 24-bit Z
            case DEPTH_IMAGE_FORMAT_R24G8_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT:
            case DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS:
            case DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT:
                return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

                // 16-bit Z w/o Stencil
            case DEPTH_IMAGE_FORMAT_R16_TYPELESS:
            case DEPTH_IMAGE_FORMAT_D16_UNORM:
            case DEPTH_IMAGE_FORMAT_R16_UNORM:
                return DXGI_FORMAT_R16_UNORM;

            default:
                assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }

        // TODO: I have no idea what I'm doing here, is this correct?
        u32 ImageHandlerDX12::GetNumElements(DepthImageFormat format)
        {
            switch (format)
            {
                case DEPTH_IMAGE_FORMAT_UNKNOWN:
                    assert(false); // This is an invalid format

                    // 32-bit Z w/ Stencil
                case DEPTH_IMAGE_FORMAT_R32G8X24_TYPELESS:          return 2;
                case DEPTH_IMAGE_FORMAT_D32_FLOAT_S8X24_UINT:       return 2;
                case DEPTH_IMAGE_FORMAT_R32_FLOAT_X8X24_TYPELESS:   return 1;
                case DEPTH_IMAGE_FORMAT_X32_TYPELESS_G8X24_UINT:    return 1;

                    // 32-bit Z w/o Stencil
                case DEPTH_IMAGE_FORMAT_R32_TYPELESS:               return 1;
                case DEPTH_IMAGE_FORMAT_D32_FLOAT:                  return 1;
                case DEPTH_IMAGE_FORMAT_R32_FLOAT:                  return 1;

                    // 24-bit Z
                case DEPTH_IMAGE_FORMAT_R24G8_TYPELESS:             return 2;
                case DEPTH_IMAGE_FORMAT_D24_UNORM_S8_UINT:          return 2;
                case DEPTH_IMAGE_FORMAT_R24_UNORM_X8_TYPELESS:      return 2;
                case DEPTH_IMAGE_FORMAT_X24_TYPELESS_G8_UINT:       return 2;

                    // 16-bit Z w/o Stencil
                case DEPTH_IMAGE_FORMAT_R16_TYPELESS:               return 1;
                case DEPTH_IMAGE_FORMAT_D16_UNORM:                  return 1;
                case DEPTH_IMAGE_FORMAT_R16_UNORM:                  return 1;

                default:
                    assert(false); // We have tried to convert a image format we don't know about, did we just add it?
            }
            return DXGI_FORMAT_UNKNOWN;
        }


    }
}