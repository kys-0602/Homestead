#include <Windows.h>

#include "Homestead/App/Application.hpp"

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE instance,
    [[maybe_unused]] HINSTANCE previousInstance,
    [[maybe_unused]] PWSTR commandLine,
    [[maybe_unused]] int showCommand) {
    Homestead::Application application;
    const Homestead::ApplicationInitializeResult result = application.Initialize(instance, showCommand);
    if (result == Homestead::ApplicationInitializeResult::Cancelled) return 0;
    if (result != Homestead::ApplicationInitializeResult::Ready) {
        MessageBoxW(nullptr, L"Homestead failed to initialize.", L"Homestead", MB_OK | MB_ICONERROR);
        return 1;
    }

    return application.Run();
}
