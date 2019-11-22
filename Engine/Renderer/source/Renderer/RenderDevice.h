#pragma once
#include <Core.h>

class Window;
class Model;

class RenderDevice
{
public:
    RenderDevice();

    virtual bool Init(Window* window, int width, int height) = 0;
    virtual void Render() = 0;
    virtual void Cleanup() = 0;
    virtual void WaitForFrame() = 0;

    // Function to register a model to be rendered this frame  TODO: Some kind of render bucket system instead
    virtual void RegisterModel(Model* model) = 0;
private:

private:

};