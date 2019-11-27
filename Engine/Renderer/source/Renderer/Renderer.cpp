#include "Renderer.h"
#include "Model.h"
#include "DX12/RenderDeviceDX12.h"

Renderer::Renderer()
    : _device(nullptr)
{

}

Renderer::~Renderer()
{

}

bool Renderer::Init(Window* window, int width, int height)
{
    _device = new RenderDeviceDX12();

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

void Renderer::Update(f32 deltaTime)
{
    Model::ConstantBuffer& constantBuffer = _model->GetConstants();
    constantBuffer.colorMultiplier += Vector4(1.0f, 1.0f, 1.0f, 0)* deltaTime;
    if (constantBuffer.colorMultiplier.x > 1.0f)
    {
        constantBuffer.colorMultiplier = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    constantBuffer.modelMatrix = _model->GetMatrix().Transposed();
}

void Renderer::Render()
{
    _device->RegisterModel(_model);
    _device->Render();
}

void Renderer::WaitForFrame()
{
    _device->WaitForFrame();
}

void Renderer::Cleanup()
{
    _device->Cleanup();
}

void Renderer::SetViewMatrix(const Matrix& viewMatrix)
{
    ViewConstantBuffer& cb = _device->GetViewConstantBuffer();
    cb.view = viewMatrix.Transposed();
}