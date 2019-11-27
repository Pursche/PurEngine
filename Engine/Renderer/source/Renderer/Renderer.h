#pragma once
#include <Core.h>

class Window;
class RenderDevice;
class RenderContext;
class Model;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Init(Window* window, int width, int height);
    void Update(f32 deltaTime);
    void Render();
    void WaitForFrame();
    void Cleanup();

    void SetViewMatrix(const Matrix& viewMatrix);

private:

private:
    RenderDevice* _device;
    Model* _model;
};