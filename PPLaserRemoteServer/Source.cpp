#include "BluetoothServer.h"
#include "UDPServer.h"
#include "GUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{   
    GUI &gui = GUI::GetInstance();
    {
        BluetoothServer bth(gui);
        UDPServer UDP(gui);
        gui.MainLoop();
    }
    return 0;
}
