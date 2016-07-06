#include "BluetoothServer.h"
#include "TCPServer.h"
#include "GUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GUI &gui = GUI::GetInstance();
    {
        BluetoothServer bth(gui);
        TCPServer tcp(gui);
        gui.MainLoop();
    }
    return 0;
}
