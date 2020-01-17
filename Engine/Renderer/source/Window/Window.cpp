#include "Window.h"
#include <Utils/StringUtils.h>

#include <cassert>

std::unordered_map<HWND, Window*> Window::_windows = std::unordered_map<HWND, Window*>();

Window::Window(HINSTANCE hInstance, int showWnd, Vector2 windowSize, bool fullScreen, std::string windowName, std::string windowTitle)
    : _windowSize(windowSize)
    , _windowName(windowName)
    , _windowTitle(windowTitle)
    , _isFullScreen(fullScreen)
{
    bool result = Initializewindow(hInstance, showWnd);
    assert(result);
    _wantsToExit = false;
}

Window::~Window()
{
    delete _swapChain;
}

// Static WndProc
LRESULT CALLBACK _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* window;
    if (Window::TryGetWindowFromHwnd(hwnd, window))
    {
        return window->WndProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Non-static WndProc
LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (MessageBox(0, L"Are you sure you want to exit?",
                L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                _wantsToExit = true;
            }
        }
        return 0;

    case WM_DESTROY:
        if (_wantsToExit)
            PostQuitMessage(0);
        else
        {
            _wantsToExit = true;
            DestroyWindow(_handle);
        }
        return 0;
    }
    return DefWindowProc(hwnd,
        msg,
        wParam,
        lParam);
}

bool Window::Initializewindow(HINSTANCE hInstance, int ShowWnd)
{
    if (_isFullScreen)
    {
        HMONITOR monitor = MonitorFromWindow(_handle, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        GetMonitorInfo(monitor, &monitorInfo);

        _windowSize.x = static_cast<f32>(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
        _windowSize.y = static_cast<f32>(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
    }

    WNDCLASSEX wc;

    std::wstring windowName = StringUtils::StringToWString(_windowName);

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = _WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = windowName.c_str();
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    std::wstring windowTitle = StringUtils::StringToWString(_windowTitle);

    _handle = CreateWindowEx(NULL,
        windowName.c_str(),
        windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<long>(_windowSize.x), static_cast<long>(_windowSize.y),
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!_handle)
    {
        MessageBox(NULL, L"Error creating window",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    _windows[_handle] = this;

    if (_isFullScreen)
    {
        SetWindowLong(_handle, GWL_STYLE, 0);
    }

    ShowWindow(_handle, ShowWnd);
    UpdateWindow(_handle);

    return true;
}

bool Window::TryGetWindowFromHwnd(HWND hwnd, Window*& window)
{
    if (_windows.find(hwnd) != _windows.end())
    {
        window = _windows[hwnd];
        return true;
    }

    return false;
}

void Window::Update(f32 /*deltaTime*/)
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window::ConfirmExit()
{
    DestroyWindow(_handle);
}

void Window::SendMessageBox(std::string title, std::string message)
{
    std::wstring wideTitle = StringUtils::StringToWString(title);
    std::wstring wideMessage = StringUtils::StringToWString(message);

    MessageBox(0, wideMessage.c_str(),
        wideTitle.c_str(), MB_OK);
}