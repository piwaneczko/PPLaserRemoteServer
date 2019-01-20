#include "GUI.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const auto hWnd = FindWindow(L"#32770", REMOTE_SERVER_TITLE);
    if (hWnd == nullptr) {
        Gui gui;
        gui.mainLoop();
    } else
        ShowWindow(hWnd, SW_RESTORE);
    return 0;
}
