#include "Homestead/Platform/Window.hpp"

#include <windowsx.h>

#include "Homestead/Input/Input.hpp"

namespace Homestead {
namespace {

constexpr wchar_t WindowClassName[] = L"HomesteadWindowClass";
constexpr wchar_t WindowTitle[] = L"Homestead";
constexpr LONG LogicalClientWidth = 320;
constexpr LONG LogicalClientHeight = 180;

bool TranslateVirtualKey(WPARAM virtualKey, PhysicalKey& key) noexcept {
    switch (virtualKey) {
    case 'W': key = PhysicalKey::W; return true;
    case 'A': key = PhysicalKey::A; return true;
    case 'S': key = PhysicalKey::S; return true;
    case 'D': key = PhysicalKey::D; return true;
    case 'E': key = PhysicalKey::E; return true;
    case 'F': key = PhysicalKey::F; return true;
    case 'I': key = PhysicalKey::I; return true;
    case 'P': key = PhysicalKey::P; return true;
    case VK_UP: key = PhysicalKey::Up; return true;
    case VK_DOWN: key = PhysicalKey::Down; return true;
    case VK_LEFT: key = PhysicalKey::Left; return true;
    case VK_RIGHT: key = PhysicalKey::Right; return true;
    case VK_SPACE: key = PhysicalKey::Space; return true;
    case VK_ESCAPE: key = PhysicalKey::Escape; return true;
    case '1': key = PhysicalKey::Digit1; return true;
    case '2': key = PhysicalKey::Digit2; return true;
    case '3': key = PhysicalKey::Digit3; return true;
    case '4': key = PhysicalKey::Digit4; return true;
    case '5': key = PhysicalKey::Digit5; return true;
    case '6': key = PhysicalKey::Digit6; return true;
    case '7': key = PhysicalKey::Digit7; return true;
    case '8': key = PhysicalKey::Digit8; return true;
    default: return false;
    }
}

} // namespace

Window::~Window() noexcept {
    Shutdown();
}

bool Window::Initialize(HINSTANCE instance, int showCommand, Input& input,
                        std::uint8_t windowScale, bool fullscreen) noexcept {
    if (handle_ != nullptr) {
        return true;
    }

    instance_ = instance;
    input_ = &input;

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
    RECT windowRectangle{0, 0, LogicalClientWidth * windowScale, LogicalClientHeight * windowScale};
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
    if (fullscreen && !ApplyDisplaySettings(windowScale, true)) {
        Shutdown();
        return false;
    }
    return true;
}

bool Window::ApplyDisplaySettings(std::uint8_t windowScale, bool fullscreen) noexcept {
    if (handle_ == nullptr || windowScale < 2 || windowScale > 4) return false;
    if (fullscreen) {
        MONITORINFO monitor{sizeof(monitor)};
        if (GetMonitorInfoW(MonitorFromWindow(handle_, MONITOR_DEFAULTTONEAREST), &monitor) == FALSE)
            return false;
        SetWindowLongPtrW(handle_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        if (SetWindowPos(handle_, HWND_TOP, monitor.rcMonitor.left, monitor.rcMonitor.top,
                         monitor.rcMonitor.right - monitor.rcMonitor.left,
                         monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                         SWP_FRAMECHANGED | SWP_SHOWWINDOW) == FALSE) return false;
    } else {
        constexpr DWORD style = WS_OVERLAPPEDWINDOW;
        RECT rectangle{0, 0, LogicalClientWidth * windowScale, LogicalClientHeight * windowScale};
        if (AdjustWindowRectEx(&rectangle, style, FALSE, 0) == FALSE) return false;
        SetWindowLongPtrW(handle_, GWL_STYLE, style);
        if (SetWindowPos(handle_, HWND_NOTOPMOST, 0, 0,
                         rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_SHOWWINDOW) == FALSE) return false;
    }
    fullscreen_ = fullscreen;
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

void Window::RequestClose() noexcept {
    if (handle_ != nullptr) PostMessageW(handle_, WM_CLOSE, 0, 0);
}

void Window::UpdateLoadingScreen(std::uint8_t completedStages, std::uint8_t animationFrame) noexcept {
    if (handle_ == nullptr || !loadingScreen_) return;
    loadingStages_ = completedStages > 5 ? 5 : completedStages;
    loadingAnimation_ = static_cast<std::uint8_t>(animationFrame % 4);
    InvalidateRect(handle_, nullptr, FALSE);
    UpdateWindow(handle_);
}

void Window::PaintLoadingScreen() noexcept {
    PAINTSTRUCT paint{};
    HDC context = BeginPaint(handle_, &paint);
    if (context == nullptr) return;
    RECT client{};
    GetClientRect(handle_, &client);
    HBRUSH background = CreateSolidBrush(RGB(31, 45, 28));
    HBRUSH empty = CreateSolidBrush(RGB(77, 62, 40));
    HBRUSH filled = CreateSolidBrush(RGB(221, 171, 79));
    FillRect(context, &client, background);
    SetBkMode(context, TRANSPARENT);
    SetTextColor(context, RGB(245, 224, 169));
    HFONT previousFont = static_cast<HFONT>(SelectObject(context, GetStockObject(DEFAULT_GUI_FONT)));
    RECT title{client.left, client.top + (client.bottom - client.top) / 2 - 42,
               client.right, client.bottom};
    DrawTextW(context, L"HOMESTEAD", -1, &title, DT_CENTER | DT_SINGLELINE);
    wchar_t loading[] = L"LOADING...";
    for (std::uint8_t index = loadingAnimation_; index < 3; ++index) loading[7 + index] = L' ';
    RECT label{client.left, title.top + 28, client.right, client.bottom};
    DrawTextW(context, loading, -1, &label, DT_CENTER | DT_SINGLELINE);
    const LONG clientWidth = client.right - client.left;
    constexpr LONG segmentWidth = 22;
    constexpr LONG segmentGap = 5;
    constexpr LONG segmentCount = 5;
    const LONG totalWidth = segmentWidth * segmentCount + segmentGap * (segmentCount - 1);
    const LONG left = client.left + (clientWidth - totalWidth) / 2;
    const LONG top = label.top + 28;
    for (LONG index = 0; index < segmentCount; ++index) {
        RECT segment{left + index * (segmentWidth + segmentGap), top,
                     left + index * (segmentWidth + segmentGap) + segmentWidth, top + 8};
        FillRect(context, &segment,
                 index < static_cast<LONG>(loadingStages_) ? filled : empty);
    }
    SelectObject(context, previousFont);
    DeleteObject(filled);
    DeleteObject(empty);
    DeleteObject(background);
    EndPaint(handle_, &paint);
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
    input_ = nullptr;
    clientWidth_ = 0;
    clientHeight_ = 0;
    minimized_ = false;
    focused_ = false;
    mouseTracking_ = false;
    fullscreen_ = false;
    loadingScreen_ = true;
    loadingStages_ = 0;
    loadingAnimation_ = 0;
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

    case WM_ERASEBKGND:
        if (loadingScreen_) return 1;
        break;

    case WM_PAINT:
        if (loadingScreen_) {
            PaintLoadingScreen();
            return 0;
        }
        break;

    case WM_SETFOCUS:
        focused_ = true;
        return 0;

    case WM_KILLFOCUS:
        focused_ = false;
        input_->OnFocusLost();
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        PhysicalKey key{};
        if (TranslateVirtualKey(wParam, key)) {
            input_->SetPhysicalKey(key, true);
            return 0;
        }
        break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP: {
        PhysicalKey key{};
        if (TranslateVirtualKey(wParam, key)) {
            input_->SetPhysicalKey(key, false);
            return 0;
        }
        break;
    }

    case WM_LBUTTONDOWN:
        SetCapture(handle_);
        input_->SetClientMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        input_->SetPhysicalKey(PhysicalKey::MouseLeft, true);
        return 0;

    case WM_LBUTTONUP:
        input_->SetClientMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        input_->SetPhysicalKey(PhysicalKey::MouseLeft, false);
        if (GetCapture() == handle_ && (wParam & MK_RBUTTON) == 0) {
            ReleaseCapture();
        }
        return 0;

    case WM_RBUTTONDOWN:
        SetCapture(handle_);
        input_->SetClientMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        input_->SetPhysicalKey(PhysicalKey::MouseRight, true);
        return 0;

    case WM_RBUTTONUP:
        input_->SetClientMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        input_->SetPhysicalKey(PhysicalKey::MouseRight, false);
        if (GetCapture() == handle_ && (wParam & MK_LBUTTON) == 0) {
            ReleaseCapture();
        }
        return 0;

    case WM_MOUSEMOVE:
        input_->SetClientMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (!mouseTracking_) {
            TRACKMOUSEEVENT tracking{};
            tracking.cbSize = sizeof(tracking);
            tracking.dwFlags = TME_LEAVE;
            tracking.hwndTrack = handle_;
            mouseTracking_ = TrackMouseEvent(&tracking) != FALSE;
        }
        return 0;

    case WM_MOUSELEAVE:
        input_->SetClientMouse(-1, -1);
        mouseTracking_ = false;
        return 0;

    default:
        return DefWindowProcW(handle_, message, wParam, lParam);
    }

    return DefWindowProcW(handle_, message, wParam, lParam);
}

} // namespace Homestead
