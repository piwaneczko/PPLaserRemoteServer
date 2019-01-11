#include "BluetoothServer.hpp"
#include "GUI.hpp"
#include "TCPServer.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GUI &gui = GUI::GetInstance();
    {
        BluetoothServer bth(gui);
        TCPServer tcp(gui);
        gui.MainLoop();
    }
    return 0;
}
