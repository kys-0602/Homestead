#include "App/Application.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Application& app = Application::GetInstance();
    if (app.Initialize(hInstance) == false) {
        MessageBox(nullptr, "애플리케이션 초기화에 실패했습니다.", "Homestead", MB_OK | MB_ICONERROR);
        return -1;
    }

    return app.Run();
}