#pragma once

#include <cstdint>
#include <string_view>
#include <Windows.h>

/// @brief 윈도우 클래스
class Window final {
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstanace;
    HWND m_hWnd;

public:
    Window() noexcept = default;
    ~Window() noexcept;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] bool Initialize(HINSTANCE hInstance, std::string_view title, int32_t width, int32_t height) noexcept;
    [[nodiscard]] HWND GetHandle() const noexcept;
};