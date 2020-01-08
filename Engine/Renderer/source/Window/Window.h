#pragma once
#include <Core.h>
#include <Windows.h>
#include <unordered_map>
#include "../Renderer/Swapchain.h"

class Window
{
    const u8 BUFFER_COUNT = 2; // TODO: Add this to a setting to allow triple buffering?

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

    u8 GetFrameBufferCount() { return BUFFER_COUNT; }
    
    // The types of some of these things will depend on the rendering backend, we store them as void pointers and let the backend keep track of what it is
    void SetSwapChain(Renderer::SwapChain* swapChain) { _swapChain = swapChain; }
    Renderer::SwapChain* GetSwapChain() { return _swapChain; }

private:
    bool Initializewindow(HINSTANCE hInstance, int ShowWnd);

private:
    bool _wantsToExit;

    std::string _windowName;
    std::string _windowTitle;
    Vector2 _windowSize;
    bool _isFullScreen;

    HWND _handle;

    Renderer::SwapChain* _swapChain;

    static std::unordered_map<HWND, Window*> _windows;
};