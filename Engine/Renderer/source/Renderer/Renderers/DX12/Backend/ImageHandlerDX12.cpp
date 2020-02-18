#include "ImageHandlerDX12.h"
#include <Utils/StringUtils.h>
#include "RenderDeviceDX12.h"
#include "CommandListHandlerDX12.h"
#include <cassert>
#include <filesystem>

namespace Renderer
{
    namespace Backend
    {
        ImageHandlerDX12::ImageHandlerDX12()
        {

        }

        ImageHandlerDX12::~ImageHandlerDX12()
        {
            for (auto& image : _images)
            {
                image.rtvDescriptorHeap.Reset();
                image.srvUavDescriptorHeap.Reset();
                image.resource.Reset();
            }
            for (auto& image : _depthImages)
            {
                image.dsvDescriptorHeap.Reset();
                image.srvDescriptorHeap.Reset();
                image.resource.Reset();
            }
            _images.clear();
            _depthImages.clear();
        }

        TextureID ImageHandlerDX12::LoadTexture(RenderDeviceDX12* device, CommandListHandlerDX12* commandListHandler, const TextureDesc& desc)
        {
            size_t nextHandle = _images.size();

            // Make sure we haven't exceeded the limit of the ImageID type, if this hits you need to change type of ImageID to something bigger
            assert(nextHandle < TextureID::MaxValue());
            using type = type_safe::underlying_type<TextureID>;

            // A texture is just an Image loaded from a file without the RTV
            Image image;
            image.isTexture = true;

            CommandListID commandListID = commandListHandler->BeginCommandList(device);
            ID3D12GraphicsCommandList* commandList =  commandListHandler->GetCommandList(commandListID);

            // So lets load the texture
            D3D12_RESOURCE_DESC textureDesc;
            int imageBytesPerRow;
            u8* imageData;
            std::wstring path = StringUtils::StringToWString(desc.path);
           
            LoadImageDataFromFile(&imageData, textureDesc, path, imageBytesPerRow);

            // Create the texture
            HRESULT result = device->_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &textureDesc, // the description of our texture
                D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
                nullptr, // used for render targets and depth/stencil buffers
                IID_PPV_ARGS(image.resource.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Could not create texture resource

            std::wstring filename = StringUtils::StringToWString(std::filesystem::path(desc.path).stem().string());
            image.resource->SetName(filename.c_str());

            // Create the upload heap
            Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap;
            UINT64 textureUploadBufferSize;
            device->_device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

            result = device->_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
                D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
                nullptr,
                IID_PPV_ARGS(uploadHeap.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Could not create texture upload heap

            uploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

            // Store vertex buffer in upload heap
            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = &imageData[0]; // pointer to our image data
            textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
            textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

            // Now we copy the upload buffer contents to the default heap
            UpdateSubresources(commandList, image.resource.Get(), uploadHeap.Get(), 0, 0, 1, &textureData);

            // Transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(image.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

            // Create the descriptor heap that will store our srv
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = 1;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            result = device->_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(image.srvUavDescriptorHeap.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Could not create SRV heap

            // Create the SRV
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = textureDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            device->_device->CreateShaderResourceView(image.resource.Get(), &srvDesc, image.srvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

            commandListHandler->EndCommandList(device, commandListID);

            _images.push_back(image);
            return TextureID(static_cast<type>(nextHandle));
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

            DXGI_FORMAT format = ToDXGIFormat(desc.format);
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
                &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(image.resource.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Failed to create commited resource

            {
                std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                debugName.append(L" Resource");
                result = image.resource->SetName(debugName.c_str());
                assert(SUCCEEDED(result)); // Failed to name commited resource
            }

            // Create RTV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            { 
                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
                rtvHeapDesc.NumDescriptors = 1;
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                result = device->_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(image.rtvDescriptorHeap.ReleaseAndGetAddressOf()));
                assert(SUCCEEDED(result)); // Failed to create RTV descriptor heap

                {
                    std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                    debugName.append(L" RTV Descriptor Heap");
                    result = image.rtvDescriptorHeap->SetName(debugName.c_str());
                    assert(SUCCEEDED(result)); // Failed to name RTV descriptor heap
                }
            }

            // Create RTV
            image.rtv = image.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            device->_device->CreateRenderTargetView(image.resource.Get(), nullptr, image.rtv);

            // Create SRV/UAV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};
                srvUavHeapDesc.NumDescriptors = 2;
                srvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                srvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                result = device->_device->CreateDescriptorHeap(&srvUavHeapDesc, IID_PPV_ARGS(image.srvUavDescriptorHeap.ReleaseAndGetAddressOf()));
                assert(SUCCEEDED(result)); // Failed to create SRV/UAV descriptor heap

                {
                    std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                    debugName.append(L" SRV UAV Descriptor Heap");
                    result = image.srvUavDescriptorHeap->SetName(debugName.c_str());
                    assert(SUCCEEDED(result)); // Failed to name SRV/UAV descriptor heap
                }
            }
            
            auto srvUavDescriptorSize = device->_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            // Create SRV
            image.srv = image.srvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            device->_device->CreateShaderResourceView(image.resource.Get(), nullptr, image.srv);

            // Create UAV
            image.uav = image.srvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            
            image.uav.ptr += srvUavDescriptorSize;
            
            device->_device->CreateUnorderedAccessView(image.resource.Get(), nullptr, nullptr, image.uav);

            _images.push_back(image);

            return ImageID(static_cast<type>(nextHandle));
        }

        DepthImageID ImageHandlerDX12::CreateDepthImage(RenderDeviceDX12* device, const DepthImageDesc& desc)
        {
            size_t nextHandle = _depthImages.size();

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

            resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(ToBaseFormat(desc.format),
                desc.dimensions.x, desc.dimensions.y,
                1, 1, SampleCountToInt(desc.sampleCount), sampleQuality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = ToDXGIFormat(desc.format);
            clearValue.DepthStencil.Depth = desc.depthClearValue;
            clearValue.DepthStencil.Stencil = desc.stencilClearValue;

            HRESULT result = device->_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
                &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(image.resource.ReleaseAndGetAddressOf()));
            assert(SUCCEEDED(result)); // Failed to create commited resource

            {
                std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                debugName.append(L" Resource");
                result = image.resource->SetName(debugName.c_str());
                assert(SUCCEEDED(result)); // Failed to name commited resource
            }

            // Create DSV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
                dsvHeapDesc.NumDescriptors = 1;
                dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                result = device->_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(image.dsvDescriptorHeap.ReleaseAndGetAddressOf()));
                assert(SUCCEEDED(result)); // Failed to create DSV descriptor heap

                {
                    std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                    debugName.append(L" DSV Descriptor Heap");
                    result = image.dsvDescriptorHeap->SetName(debugName.c_str());
                    assert(SUCCEEDED(result)); // Failed to name DSV descriptor heap
                }
            }

            // Create DSV
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = ToDSVFormat(desc.format);

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
            device->_device->CreateDepthStencilView(image.resource.Get(), &dsvDesc, image.dsv);

            // Create SRV Descriptor heap TODO: Figure out a way to manage and combine descriptor heaps since it should cache better
            {
                D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
                srvHeapDesc.NumDescriptors = 1;
                srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                result = device->_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(image.srvDescriptorHeap.ReleaseAndGetAddressOf()));
                assert(SUCCEEDED(result)); // Failed to create SRV descriptor heap

                {
                    std::wstring debugName = StringUtils::StringToWString(desc.debugName);
                    debugName.append(L" SRV Descriptor Heap");
                    result = image.srvDescriptorHeap->SetName(debugName.c_str());
                    assert(SUCCEEDED(result)); // Failed to name SRV descriptor heap
                }
            }

            // Create SRV
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = ToDepthFormat(desc.format);

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
            device->_device->CreateShaderResourceView(image.resource.Get(), &srvDesc, image.srv);

            _depthImages.push_back(image);

            return DepthImageID(static_cast<type>(nextHandle));
        }

        const ImageDesc& ImageHandlerDX12::GetDescriptor(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            // Lets make sure this is a rendertarget and not a texture
            assert(!_images[static_cast<type>(id)].isTexture);

            return _images[static_cast<type>(id)].desc;
        }

        const DepthImageDesc& ImageHandlerDX12::GetDescriptor(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].desc;
        }

        DXGI_FORMAT ImageHandlerDX12::GetDXGIFormat(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            // Lets make sure this is a rendertarget and not a texture
            assert(!_images[static_cast<type>(id)].isTexture);

            return ToDXGIFormat(_images[static_cast<type>(id)].desc.format);
        }

        DXGI_FORMAT ImageHandlerDX12::GetDXGIFormat(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));

            // Lets make sure this is a rendertarget and not a texture
            assert(!_images[static_cast<type>(id)].isTexture);

            return ToDXGIFormat(_depthImages[static_cast<type>(id)].desc.format);
        }

        ID3D12Resource* ImageHandlerDX12::GetResource(const TextureID id)
        {
            using type = type_safe::underlying_type<TextureID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));
            return _images[static_cast<type>(id)].resource.Get();
        }

        ID3D12Resource* ImageHandlerDX12::GetResource(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));
            return _images[static_cast<type>(id)].resource.Get();
        }

        ID3D12Resource* ImageHandlerDX12::GetResource(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].resource.Get();
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetSRV(const TextureID id)
        {
            using type = type_safe::underlying_type<TextureID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));
            return _images[static_cast<type>(id)].srv;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetRTV(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            // Lets make sure this is a rendertarget and not a texture
            assert(!_images[static_cast<type>(id)].isTexture);

            return _images[static_cast<type>(id)].rtv;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetSRV(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            return _images[static_cast<type>(id)].srv;
        }

        ID3D12DescriptorHeap* ImageHandlerDX12::GetSRVDescriptorHeap(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            return _images[static_cast<type>(id)].srvUavDescriptorHeap.Get();
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetUAV(const ImageID id)
        {
            using type = type_safe::underlying_type<ImageID>;

            // Lets make sure this id exists
            assert(_images.size() > static_cast<type>(id));

            // Lets make sure this is a rendertarget and not a texture
            assert(!_images[static_cast<type>(id)].isTexture);

            return _images[static_cast<type>(id)].uav;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetDSV(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].dsv;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE ImageHandlerDX12::GetSRV(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].srv;
        }

        ID3D12DescriptorHeap* ImageHandlerDX12::GetSRVDescriptorHeap(const DepthImageID id)
        {
            using type = type_safe::underlying_type<DepthImageID>;

            // Lets make sure this id exists
            assert(_depthImages.size() > static_cast<type>(id));
            return _depthImages[static_cast<type>(id)].srvDescriptorHeap.Get();
        }

        void ImageHandlerDX12::LoadImageDataFromFile(u8** imageData, D3D12_RESOURCE_DESC& resourceDescription, std::wstring fileName, int& bytesPerRow)
        {
            HRESULT hr;

            // we only need one instance of the imaging factory to create decoders and frames
            static IWICImagingFactory* wicFactory;

            // reset decoder, frame and converter since these will be different for each image we load
            IWICBitmapDecoder* wicDecoder = NULL;
            IWICBitmapFrameDecode* wicFrame = NULL;
            IWICFormatConverter* wicConverter = NULL;

            bool imageConverted = false;

            if (wicFactory == NULL)
            {
                // Initialize the COM library
                CoInitialize(NULL);

                // create the WIC factory
                hr = CoCreateInstance(
                    CLSID_WICImagingFactory,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&wicFactory)
                );
                assert(SUCCEEDED(hr));
            }

            // load a decoder for the image
            hr = wicFactory->CreateDecoderFromFilename(
                fileName.c_str(),               // Image we want to load in
                NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
                GENERIC_READ,                    // We want to read from this file
                WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
                &wicDecoder                      // the wic decoder to be created
            );
            assert(SUCCEEDED(hr));

            // get image from decoder (this will decode the "frame")
            hr = wicDecoder->GetFrame(0, &wicFrame);
            assert(SUCCEEDED(hr));

            // get wic pixel format of image
            WICPixelFormatGUID pixelFormat;
            hr = wicFrame->GetPixelFormat(&pixelFormat);
            assert(SUCCEEDED(hr));

            // get size of image
            UINT textureWidth, textureHeight;
            hr = wicFrame->GetSize(&textureWidth, &textureHeight);
            assert(SUCCEEDED(hr));

            // we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
            // out how to implement the support yourself

            // convert wic pixel format to dxgi pixel format
            DXGI_FORMAT dxgiFormat = ToDXGIFormat(pixelFormat);

            // if the format of the image is not a supported dxgi format, try to convert it
            if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            {
                // get a dxgi compatible wic format from the current image format
                WICPixelFormatGUID convertToPixelFormat = ToCompatibleWICFormat(pixelFormat);

                // return if no dxgi compatible format was found
                assert(convertToPixelFormat != GUID_WICPixelFormatDontCare);

                // set the dxgi format
                dxgiFormat = ToDXGIFormat(convertToPixelFormat);
                assert(dxgiFormat != DXGI_FORMAT_UNKNOWN);

                // create the format converter
                hr = wicFactory->CreateFormatConverter(&wicConverter);
                assert(SUCCEEDED(hr));

                // make sure we can convert to the dxgi compatible format
                BOOL canConvert = FALSE;
                hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
                assert(SUCCEEDED(hr));
                assert(canConvert);

                // do the conversion (wicConverter will contain the converted image)
                hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
                assert(SUCCEEDED(hr));

                // this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
                imageConverted = true;
            }

            int bitsPerPixel = ToBitsPerPixel(dxgiFormat); // number of bits per pixel
            bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
            int imageSize = bytesPerRow * textureHeight; // total image size in bytes

            // allocate enough memory for the raw image data, and set imageData to point to that memory
            *imageData = (BYTE*)malloc(imageSize);

            // copy (decoded) raw image data into the newly allocated memory (imageData)
            if (imageConverted)
            {
                // if image format needed to be converted, the wic converter will contain the converted image
                hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
                assert(SUCCEEDED(hr));
            }
            else
            {
                // no need to convert, just copy data from the wic frame
                hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
                assert(SUCCEEDED(hr));
            }

            // now describe the texture with the information we have obtained from the image
            resourceDescription = {};
            resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
            resourceDescription.Width = textureWidth; // width of the texture
            resourceDescription.Height = textureHeight; // height of the texture
            resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
            resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
            resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
            resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
            resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
            resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
            resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags
        }

        DXGI_FORMAT ImageHandlerDX12::ToDXGIFormat(ImageFormat format)
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

        DXGI_FORMAT ImageHandlerDX12::ToBaseFormat(DepthImageFormat format)
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

        DXGI_FORMAT ImageHandlerDX12::ToDXGIFormat(DepthImageFormat format)
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

        DXGI_FORMAT ImageHandlerDX12::ToDSVFormat(DepthImageFormat format)
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

        DXGI_FORMAT ImageHandlerDX12::ToDepthFormat(DepthImageFormat format)
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

        u32 ImageHandlerDX12::ToNumElements(DepthImageFormat format)
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

        ::DXGI_FORMAT ImageHandlerDX12::ToDXGIFormat(WICPixelFormatGUID& wicFormatGUID)
        {
            // get the dxgi format equivilent of a wic format
            if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

            else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
            else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

            return DXGI_FORMAT_UNKNOWN;
        }

        WICPixelFormatGUID ImageHandlerDX12::ToCompatibleWICFormat(WICPixelFormatGUID& wicFormatGUID)
        {
            if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
            else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
            else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
            else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
            else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
            else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
            else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
            else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

            assert(false); // Could not find a DXGI compatible wic format
            return GUID_WICPixelFormatDontCare;
        }

        int ImageHandlerDX12::ToBitsPerPixel(DXGI_FORMAT& dxgiFormat)
        {
            // get the number of bits per pixel for a dxgi format
            if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
            else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
            else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
            else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
            else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
            else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
            else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

            else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
            else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
            else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
            else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
            else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
            else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
            else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
            else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
            
            assert(false); // Tried to find BPP of unsupported DXGI format
            return 0;
        }
    }
}