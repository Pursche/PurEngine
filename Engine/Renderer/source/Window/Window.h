#pragma once
#include <Core.h>
#include <Windows.h>
#include <unordered_map>

class Window
{
public:
    Window(HINSTANCE hInstance, int showWnd, Vector2 windowSize, bool fullScreen = false, std::string windowName = "PurEngine", std::string windowTitle = "PurEngine");
    ~Window();

    void Update(f32 deltaTime);

    bool WantsToExit() { return _wantsToExit; }
    void ConfirmExit();

    std::string GetWindowName() { return _windowName; }
    Vector2 GetWindowSize() { return _windowSize; }
    bool IsFullScreen() { return _isFullScreen; }

    HWND GetHandle() { return _handle; }
    static bool TryGetWindowFromHwnd(HWND hwnd, Window*& window);

    // callback function for windows messages
    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void SendMessageBox(std::string title, std::string message);
private:
    bool Initializewindow(HINSTANCE hInstance, int ShowWnd);

private:
    bool _wantsToExit;

    std::string _windowName;
    std::string _windowTitle;
    Vector2 _windowSize;
    bool _isFullScreen;

    HWND _handle;

    static std::unordered_map<HWND, Window*> _windows;
};