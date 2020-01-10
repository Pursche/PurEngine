#pragma once
#include <Core.h>
#include <vector>
#include "../PipelineHandle.h"
#include "../ShaderHandle.h"

class OldRenderDeviceDX12;
class ShaderHandler;

struct ID3D12PipelineState;
struct ID3D12RootSignature;
struct DXGI_SAMPLE_DESC;

enum PipelineType
{
    PIPELINE_VERTEX,
    PIPELINE_COMPUTE
};
struct PipelineDesc
{
    PipelineType type = PIPELINE_VERTEX;
    ShaderHandleVertex vertexShader = ShaderHandleVertex::Invalid();
    ShaderHandlePixel pixelShader = ShaderHandlePixel::Invalid();
    ShaderHandleCompute computeShader = ShaderHandleCompute::Invalid();

    DXGI_SAMPLE_DESC* sampleDesc = NULL;
};

class PipelineHandler
{
public:
    PipelineHandler(ShaderHandler* shaderHandler) { _shaderHandler = shaderHandler; }
    ~PipelineHandler();

    bool CreatePipeline(OldRenderDeviceDX12* device, PipelineDesc& desc, PipelineHandle& handle);

    ID3D12PipelineState* GetPSO(PipelineHandle& handle);
    ID3D12RootSignature* GetRootSignature(PipelineHandle& handle);
private:
    struct LoadedPipeline
    {
        PipelineHandle handle;
        ID3D12PipelineState* pso;
        ID3D12RootSignature* rootSignature;
        OldRenderDeviceDX12* device;
    };

    typedef std::vector<char> ShaderBinary;

    ShaderBinary ReadFile(const std::string& filename);

private:
    std::vector<LoadedPipeline> _pipelines;
    ShaderHandler* _shaderHandler;

};