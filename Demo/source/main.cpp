#include <Windows.h>
#include <Core.h>

#include <Utils/Timer.h>
#include <Window/Window.h>
#include <Renderer/Renderer.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    PSTR /*lpCmdLine*/, INT nCmdShow)
{
    Vector3 a = Vector3(1, 1, 1);
    Vector3 b = Vector3(2, 2, 2);
    Vector3 c = a - b;

    const int width = 1280;
    const int height = 720;

    Window mainWindow(hInstance, nCmdShow, Vector2(width, height));

    Renderer renderer;
    if (!renderer.Init(&mainWindow, width, height))
    {
        mainWindow.SendMessageBox("Error", "Failed to initialize direct3d 12");
    }

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
        renderer.Update(deltaTime);
        renderer.Render();
    }

    return 0;
}