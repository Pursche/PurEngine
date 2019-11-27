#include <Windows.h>
#include <Core.h>

#include <Utils/Timer.h>
#include <Window/Window.h>
#include <Renderer/Renderer.h>

#include "Camera.h"

INT WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    PSTR /*lpCmdLine*/, INT nCmdShow)
{
    const int width = 1280;
    const int height = 720;

    Window mainWindow(hInstance, nCmdShow, Vector2(width, height));

    Renderer renderer;
    if (!renderer.Init(&mainWindow, width, height))
    {
        mainWindow.SendMessageBox("Error", "Failed to initialize direct3d 12");
    }

    Camera camera(Vector3(0,0,-5));

    Timer timer;
    while (true)
    {
        f32 deltaTime = timer.GetDeltaTime();
        timer.Tick();
        

        if (mainWindow.WantsToExit())
        {
            renderer.Cleanup();
            mainWindow.ConfirmExit();
            break;
        }
        mainWindow.Update(deltaTime);

        camera.Update(deltaTime);

        renderer.SetViewMatrix(camera.GetViewMatrix().Inverted());
        renderer.Update(deltaTime);
        renderer.Render();
    }

    return 0;
}