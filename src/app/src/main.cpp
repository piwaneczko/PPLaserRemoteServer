#include "BluetoothServer.hpp"
#include "GUI.hpp"
#include "TCPServer.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const auto hWnd = FindWindow(L"#32770", REMOTE_SERVER_TITLE);
    if (hWnd == nullptr) {
        auto& gui = Gui::getInstance();
        {
            BluetoothServer bth(gui);
            TCPServer tcp(gui);
            gui.mainLoop();
        }
    } else
        ShowWindow(hWnd, SW_RESTORE);
    return 0;
}
