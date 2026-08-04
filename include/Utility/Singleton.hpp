#pragma once

/// @brief 싱글톤 템플릿 클래스
/// @tparam T 클래스 타입
template<typename T>
class Singleton {
protected:
    Singleton() noexcept = default;
    ~Singleton() noexcept = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    /// @brief 인스턴스를 취득합니다.
    /// @return 인스턴스
    [[nodiscard]] static T& GetInstance() noexcept {
        static T instance;
        return instance;
    }
};