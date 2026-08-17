#include "Homestead/Platform/Window.hpp"

namespace Homestead {
namespace {

constexpr wchar_t WindowClassName[] = L"HomesteadWindowClass";
constexpr wchar_t WindowTitle[] = L"Homestead";
constexpr LONG InitialClientWidth = 1280;
constexpr LONG InitialClientHeight = 720;

} // namespace

Window::~Window() noexcept {
    Shutdown();
}

bool Window::Initialize(HINSTANCE instance, int showCommand) noexcept {
    if (handle_ != nullptr) {
        return true;
    }

    instance_ = instance;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WindowClassName;

    if (RegisterClassExW(&windowClass) == 0) {
        instance_ = nullptr;
        return false;
    }
    classRegistered_ = true;

    constexpr DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    constexpr DWORD extendedStyle = 0;
    RECT windowRectangle{0, 0, InitialClientWidth, InitialClientHeight};
    if (AdjustWindowRectEx(&windowRectangle, windowStyle, FALSE, extendedStyle) == FALSE) {
        Shutdown();
        return false;
    }

    handle_ = CreateWindowExW(
        extendedStyle,
        WindowClassName,
        WindowTitle,
        windowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRectangle.right - windowRectangle.left,
        windowRectangle.bottom - windowRectangle.top,
        nullptr,
        nullptr,
        instance_,
        this);

    if (handle_ == nullptr) {
        Shutdown();
        return false;
    }

    RECT clientRectangle{};
    if (GetClientRect(handle_, &clientRectangle) != FALSE) {
        clientWidth_ = static_cast<std::uint32_t>(clientRectangle.right - clientRectangle.left);
        clientHeight_ = static_cast<std::uint32_t>(clientRectangle.bottom - clientRectangle.top);
    }

    ShowWindow(handle_, showCommand);
    UpdateWindow(handle_);
    return true;
}

bool Window::ProcessMessages() noexcept {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        if (message.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return handle_ != nullptr;
}

void Window::Shutdown() noexcept {
    if (handle_ != nullptr) {
        DestroyWindow(handle_);
        handle_ = nullptr;
    }

    if (classRegistered_) {
        UnregisterClassW(WindowClassName, instance_);
        classRegistered_ = false;
    }

    instance_ = nullptr;
    clientWidth_ = 0;
    clientHeight_ = 0;
    minimized_ = false;
    focused_ = false;
}

LRESULT CALLBACK Window::WindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) noexcept {
    Window* owner = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        owner = static_cast<Window*>(create->lpCreateParams);
        owner->handle_ = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(owner));
    }

    if (owner != nullptr) {
        return owner->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(handle_);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCDESTROY: {
        const LRESULT result = DefWindowProcW(handle_, message, wParam, lParam);
        handle_ = nullptr;
        return result;
    }

    case WM_SIZE:
        clientWidth_ = static_cast<std::uint32_t>(LOWORD(lParam));
        clientHeight_ = static_cast<std::uint32_t>(HIWORD(lParam));
        minimized_ = wParam == SIZE_MINIMIZED || clientWidth_ == 0 || clientHeight_ == 0;
        return 0;

    case WM_SETFOCUS:
        focused_ = true;
        return 0;

    case WM_KILLFOCUS:
        focused_ = false;
        return 0;

    default:
        return DefWindowProcW(handle_, message, wParam, lParam);
    }
}

} // namespace Homestead
