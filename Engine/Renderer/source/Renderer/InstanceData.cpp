#include "InstanceData.h"
#include "Renderer.h"

namespace Renderer
{
    InstanceData::InstanceData(Renderer* renderer)
        : modelCB(renderer->CreateConstantBuffer<ModelConstantBuffer>())
    {

    }

    void InstanceData::Apply(u32 frameIndex)
    {
        modelCB.resource.colorMultiplier = colorMultiplier;
        modelCB.resource.modelMatrix = Matrix();
        
        modelCB.resource.modelMatrix.Scale(scale, Matrix::MultiplicationType::POST);
        modelCB.resource.modelMatrix.RotateXYZ(rotation, Matrix::MultiplicationType::POST);
        modelCB.resource.modelMatrix.pos = position;

        modelCB.resource.modelMatrix.Transpose();
        
        modelCB.Apply(frameIndex);
    }

    void* InstanceData::GetGPUResource(u32 frameIndex)
    {
        return modelCB.GetGPUResource(frameIndex);
    }
}