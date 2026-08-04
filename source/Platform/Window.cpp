#include "Platform/Window.hpp"

/// @brief 윈도우 프리시저
/// @param hWnd 윈도우 핸들
/// @param uMsg 메시지
/// @param wParam 메시지 파라미터
/// @param lParam 메시지 파라미터
/// @return 
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE: {
            DestroyWindow(hWnd);
            return 0;
        } break;

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;

        default: break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// @brief 소멸자
Window::~Window() noexcept {
    if (m_hWnd != nullptr) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
    
    m_hInstanace = nullptr;
}

/// @brief 초기화
/// @param hInstance 프로그램의 인스턴스 핸들
/// @param title 타이틀
/// @param width 가로 길이
/// @param height 세로 길이
/// @return 성공(true), 실패(false)
bool Window::Initialize(HINSTANCE hInstance, std::string_view title, int32_t width, int32_t height) noexcept {
    m_hInstanace = hInstance;

    WNDCLASSEX wndExCls = {};
    wndExCls.cbSize = sizeof(WNDCLASSEX);
    wndExCls.style = CS_HREDRAW | CS_VREDRAW;
    wndExCls.lpfnWndProc = WndProc;
    wndExCls.hInstance = m_hInstanace;
    wndExCls.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndExCls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wndExCls.lpszClassName = title.data();

    if (RegisterClassEx(&wndExCls) == 0) {
        return false;
    }

    RECT windowRect = { 0, 0, width, height };
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    if (AdjustWindowRect(&windowRect, dwStyle, FALSE) == 0) {
        return false;
    }

    m_hWnd = CreateWindowEx(0, title.data(), title.data(), dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, m_hInstanace, this);
    if (m_hWnd == nullptr) {
        return false;
    }

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

/// @brief 핸들을 취득합니다.
/// @return 핸들
HWND Window::GetHandle() const noexcept {
    return m_hWnd;
}