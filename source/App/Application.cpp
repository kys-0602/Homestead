#include "App/Application.hpp"

/// @brief 초기화를 수행합니다.
/// @param hInstance 프로그램의 인스턴스 핸들
/// @return 성공(true), 실패(false)
bool Application::Initialize(HINSTANCE hInstance) noexcept {
    if (m_Window.Initialize(hInstance, "HOMESTEAD", 1280, 720) == false) {
        return false;
    }

    return true;
}

/// @brief 애플리케이션을 구동합니다.
/// @return 성공(0), 실패(그 외)
int32_t Application::Run() noexcept {
    MSG msg;

    while (true) {
        BOOL result = GetMessage(&msg, nullptr, 0, 0);
        if (result == 0) return (int)msg.wParam;
        if (result == -1) return -1;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}