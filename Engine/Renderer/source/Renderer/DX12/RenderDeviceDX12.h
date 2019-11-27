#pragma once
#include "../RenderSettings.h"
#include "../RenderDevice.h"
#include "../PipelineHandle.h"
#include "d3dx12.h"
#include <vector>

struct IDXGISwapChain3;

class ShaderHandler;
class PipelineHandler;

class RenderDeviceDX12 : public RenderDevice
{
public:
    RenderDeviceDX12();

    bool Init(Window* window, int width, int height) override;
    void Render() override;
    void WaitForFrame() override;

    void Cleanup() override;

    void RegisterModel(Model* model) override;
    void InitModel(Model* model);

    ID3D12Device* GetDevice() { return _device; }
    DXGI_SAMPLE_DESC* GetSampleDesc() { return _sampleDesc; }

    ID3D12GraphicsCommandList* GetCopyCommandList() { return _copyCommandList; }
    ID3D12DescriptorHeap* GetMainDescriptorHeap() { return _mainDescriptorHeap[_frameIndex]; }
private:
    void EnableShaderBasedValidation();
    
    void UpdatePipeline();
    void UpdateViewCB();

private:
    ID3D12Device* _device;

    DXGI_SAMPLE_DESC* _sampleDesc;
    IDXGISwapChain3* _swapChain;
    ID3D12CommandQueue* _commandQueue;

    ID3D12DescriptorHeap* _rtvDescriptorHeap;
    u32 _rtvDescriptorSize;

    ID3D12DescriptorHeap* _mainDescriptorHeap[frameBufferCount];

    ID3D12Resource* _renderTargets[frameBufferCount];

    ID3D12CommandAllocator* _commandAllocator[frameBufferCount];
    ID3D12CommandAllocator* _copyCommandAllocator[frameBufferCount];

    ID3D12GraphicsCommandList* _commandList;
    ID3D12GraphicsCommandList* _copyCommandList;

    PipelineHandle _pipelineHandle;

    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    ID3D12Fence* _fence[2];
    u64 _fenceValue[2];
    void* _fenceEvent;

    u32 _frameIndex;

    std::vector<Model*> _modelsToRender;

    ShaderHandler* _shaderHandler;
    PipelineHandler* _pipelineHandler;

    ID3D12Resource* _viewConstantBufferUploadHeap[frameBufferCount];
    ViewConstantBuffer* _viewConstantBufferGPUAddress[frameBufferCount];

};