#pragma once

#include <Windows.h>

#include <cstdint>

namespace Homestead {

class Input;

class Window final {
public:
    Window() = default;
    ~Window() noexcept;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool Initialize(HINSTANCE instance, int showCommand, Input& input,
                                  std::uint8_t windowScale, bool fullscreen) noexcept;
    [[nodiscard]] bool ApplyDisplaySettings(std::uint8_t windowScale, bool fullscreen) noexcept;
    [[nodiscard]] bool ProcessMessages() noexcept;
    void RequestClose() noexcept;
    void UpdateLoadingScreen(std::uint8_t completedStages, std::uint8_t animationFrame) noexcept;
    void EndLoadingScreen() noexcept { loadingScreen_ = false; }
    void Shutdown() noexcept;

    [[nodiscard]] HWND Handle() const noexcept { return handle_; }
    [[nodiscard]] std::uint32_t ClientWidth() const noexcept { return clientWidth_; }
    [[nodiscard]] std::uint32_t ClientHeight() const noexcept { return clientHeight_; }
    [[nodiscard]] bool IsMinimized() const noexcept { return minimized_; }
    [[nodiscard]] bool HasFocus() const noexcept { return focused_; }

private:
    static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    void PaintLoadingScreen() noexcept;

    HINSTANCE instance_ = nullptr;
    HWND handle_ = nullptr;
    Input* input_ = nullptr;
    std::uint32_t clientWidth_ = 0;
    std::uint32_t clientHeight_ = 0;
    bool classRegistered_ = false;
    bool minimized_ = false;
    bool focused_ = false;
    bool mouseTracking_ = false;
    bool fullscreen_ = false;
    bool loadingScreen_ = true;
    std::uint8_t loadingStages_ = 0;
    std::uint8_t loadingAnimation_ = 0;
};

} // namespace Homestead
