#pragma once

#include "Platform/Window.hpp"
#include "Utility/Singleton.hpp"

/// @brief 애플리케이션 클래스
class Application final : public Singleton<Application> {
    friend class Singleton<Application>;

private:
    Window m_Window;

    Application() noexcept = default;
    ~Application() noexcept = default;

public:
    [[nodiscard]] bool Initialize(HINSTANCE hInstance) noexcept;
    [[nodiscard]] int32_t Run() noexcept;
};