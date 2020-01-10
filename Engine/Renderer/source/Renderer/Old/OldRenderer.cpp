#include "OldRenderer.h"
#include "Model.h"
#include "DX12/RenderDeviceDX12.h"

OldRenderer::OldRenderer()
    : _device(nullptr)
{

}

OldRenderer::~OldRenderer()
{

}

bool OldRenderer::Init(Window* window, int width, int height)
{
    _device = new OldRenderDeviceDX12();

    bool result = _device->Init(window, width, height);
    if (!result)
    {
        return false;
    }

    _model = new Model();
    _model->LoadFromFile("Data/models/Cube.purmodel");
    //_model->MakeQuad();
    return true;
}

void OldRenderer::Update(f32 deltaTime)
{
    Model::ConstantBuffer& constantBuffer = _model->GetConstants();
    constantBuffer.colorMultiplier += Vector4(1.0f, 1.0f, 1.0f, 0)* deltaTime;
    if (constantBuffer.colorMultiplier.x > 1.0f)
    {
        constantBuffer.colorMultiplier = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    constantBuffer.modelMatrix = _model->GetMatrix().Transposed();
}

void OldRenderer::Render()
{
    _device->RegisterModel(_model);
    _device->Render();
}

void OldRenderer::WaitForFrame()
{
    _device->WaitForFrame();
}

void OldRenderer::Cleanup()
{
    _device->Cleanup();
}

void OldRenderer::SetViewMatrix(const Matrix& viewMatrix)
{
    ViewConstantBuffer& cb = _device->GetViewConstantBuffer();
    cb.view = viewMatrix.Transposed();
}