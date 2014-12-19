#include <iostream>
#include <cstdint>
#include <Windows.h>
#include <Strsafe.h>
#include "resource.h"


#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window

#define SWM_SHOW	WM_APP + 1//	show the window
#define SWM_HIDE	WM_APP + 2//	hide the window
#define SWM_EXIT	WM_APP + 3//	close the window    


class GUI {
private:
    NOTIFYICONDATA niData;
    HINSTANCE hInstance;
    friend void ShowContextMenu(HWND hWnd);
    friend LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    GUI(HINSTANCE hInstance) {
        this->hInstance = hInstance;
        LPSTR wClass = "PPLaserRemoteServer";

        WNDCLASSEX wc;

        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_LASER_ICON));
        wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_LASER_ICON));
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = wClass;

        // REJESTROWANIE KLASY OKNA
        if (!RegisterClassEx(&wc))
        {
            exit(1);
        }
        HWND hwnd = CreateWindow(wClass, "Laser Remote Server", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hInstance, NULL);

        if (hwnd == NULL) exit(1);
        else ShowWindow(hwnd, SW_SHOWDEFAULT);

        SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);

        // Declare NOTIFYICONDATA details. 
        // Error handling is omitted here for brevity. Do not omit it in your code.

        niData.cbSize = sizeof(niData);
        niData.hWnd = hwnd;
        niData.uVersion = NOTIFYICON_VERSION_4;
        niData.uFlags = NIF_ICON | NIF_TIP | NIF_INFO | NIF_MESSAGE;
        niData.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LASER_ICON));
        niData.uCallbackMessage = SWM_TRAYMSG;

        // This text will be shown as the icon's tip.
        StringCchCopy(niData.szTip, ARRAYSIZE(niData.szTip), "PowerPoint Remote Server");


        // Show the notification.
        Shell_NotifyIcon(NIM_ADD, &niData);

        // This text will be shown as the icon's info.
        StringCchCopy(niData.szInfo, ARRAYSIZE(niData.szInfo), "PowerPoint Remote Server Started");
        // Show the notification.;
        Shell_NotifyIcon(NIM_MODIFY, &niData);
    }
    ~GUI() {
        Shell_NotifyIcon(NIM_DELETE, &niData);
    }
public:
    static GUI& GetInstance(HINSTANCE hInstance) {
        static GUI gui(hInstance);
        return gui;
    }
    uint32_t MainLoop() {
        // Pêtla komunikatów
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return msg.wParam;
    }
};
void ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        if (IsWindowVisible(hWnd))
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, "Hide");
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, "Show");
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, "Exit");

        // note:	must set window to the foreground or the
        //			menu won't disappear when it should
        SetForegroundWindow(hWnd);

        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,
            pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    GUI *gui = (GUI *)GetWindowLong(hWnd, GWL_USERDATA);
    switch (msg)
    { 
    case SWM_TRAYMSG:
        switch (lParam)
        {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hWnd, SW_RESTORE);
            break;
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            ShowContextMenu(hWnd);
        }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 1;
        }
        else if ((wParam & 0xFFF0) == SC_CLOSE)
        {
            DestroyWindow(hWnd);
            return 1;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case SWM_SHOW:
            ShowWindow(hWnd, SW_RESTORE);
            if (gui) {
                gui->niData.hIcon = LoadIcon(gui->hInstance, MAKEINTRESOURCE(IDI_LASER_ICON_ON));
                Shell_NotifyIcon(NIM_MODIFY, &gui->niData);
            }
            break;
        case SWM_HIDE:
        case IDOK:
            ShowWindow(hWnd, SW_HIDE);
            if (gui) {
                gui->niData.hIcon = LoadIcon(gui->hInstance, MAKEINTRESOURCE(IDI_LASER_ICON_OFF));
                Shell_NotifyIcon(NIM_MODIFY, &gui->niData);
            }
            break;
        case SWM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{   
    GUI &gui = GUI::GetInstance(hInstance);

    uint32_t guiResult = gui.MainLoop();

    return guiResult;
}
