#pragma once
#include <Core.h>

class Window;
class RenderDevice;
class RenderContext;
class Model;

class OldRenderer
{
public:
    OldRenderer();
    ~OldRenderer();

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