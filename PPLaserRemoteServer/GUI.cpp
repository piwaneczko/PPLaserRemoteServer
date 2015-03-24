/**
* \brief ¯ród³o klasy interfejsu graficznego
* \file GUI.cpp
* \author Pawe³ Iwaneczko
*/

#include "GUI.h"
#include <assert.h>
#include <math.h>

using namespace std;

#define SWM_TRAYMSG	WM_APP          /**< The message ID sent to our window */
#define SWM_SHOW	WM_APP + 1      /**< Show the window                   */
#define SWM_HIDE	WM_APP + 2      /**< Hide the window                   */
#define SWM_EXIT	WM_APP + 3      /**< Close the window                  */

enum msg_type_t {
    msg_type_key = 0,
    msg_type_laser = 1,
    msg_type_gesture = 2,
    msg_type_gyro = 3
};

enum msg_key_type_t {
    msg_key_type_esc = 0,
    msg_key_type_f5 = 1,
    msg_key_type_shf5 = 2,
    msg_key_type_prev = 3,
    msg_key_type_next = 4,
    msg_key_type_left_down = 5,
    msg_key_type_left_up = 6,
    msg_key_type_right_down = 7,
    msg_key_type_right_up = 8,
    msg_key_type_volume_down = 9,
    msg_key_type_volume_up = 10
};

#define CRC_INIT		0xFFFFu
#define CRC_VALID		0xF0B8u

/**
 * Aktualizacja sumy kontrolnej
 * \param crc Aktualna wartoœæ sumy kontrolnej
 * \param byte Bajt do dodania do sumy
 * \return Nowa wartoœæ sumy kontrolnej
 */
static uint16_t CrcUpdate(uint16_t crc, uint8_t byte)
{
    uint16_t h;

    h = (uint8_t)(crc ^ byte);
    h ^= (uint8_t)(uint16_t)(h << 4);
    crc >>= 8;
    crc ^= (uint16_t)(h << 8);
    crc ^= (uint16_t)(h << 3);
    crc ^= (uint16_t)(h >> 4);
    return crc;
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (GetWindowLong(hWnd, GWL_USERDATA) != NULL) {
        GUI &gui = *(GUI *)GetWindowLong(hWnd, GWL_USERDATA);
        switch (msg)
        {
        case SWM_TRAYMSG:
            switch (lParam)
            {
            case WM_LBUTTONDBLCLK:
                if (IsWindowVisible(hWnd)) {
                    ShowWindow(hWnd, SW_HIDE);
                }
                else {
                    ShowWindow(hWnd, SW_RESTORE);
                    UpdateWindow(hWnd);
                }
                return 1;
            case WM_RBUTTONDOWN:
            case WM_CONTEXTMENU:
                gui.ShowContextMenu(hWnd);
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
                break;
            case SWM_HIDE:
            case IDOK:
                ShowWindow(hWnd, SW_HIDE);
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
    }
    else {

        // nie ustalono klasy GUI
        switch (msg)
        {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }

    return 0;
}
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    GUI *gui;
    // uzyskanie wskaŸnika na glemm
    gui = (GUI *)dwData;

    if (gui != NULL) {
        ScrrenRect rect = {
            lprcMonitor->left,
            lprcMonitor->top,
            lprcMonitor->right - lprcMonitor->left,
            lprcMonitor->bottom - lprcMonitor->top
        };
        gui->screens.push_back(rect);
    }

    return TRUE;
}

GUI::GUI() {
    hInstance = GetModuleHandle(0);
    hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc);

    HICON hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_LASER_ICON),
        IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    if (hWnd == NULL) exit(1);

    SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);

    // Declare NOTIFYICONDATA details. 
    // Error handling is omitted here for brevity. Do not omit it in your code.

    niData.cbSize = sizeof(niData);
    niData.hWnd = hWnd;
    niData.uVersion = NOTIFYICON_VERSION_4;
    niData.uFlags = NIF_ICON | NIF_TIP | NIF_INFO | NIF_MESSAGE;
    niData.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_LASER_ICON),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    niData.uCallbackMessage = SWM_TRAYMSG;

    // This text will be shown as the icon's tip.
    wsprintf(niData.szTip, L"PowerPoint Remote Server");

    // Show the notification.
    Shell_NotifyIcon(NIM_ADD, &niData);

    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon))
        niData.hIcon = NULL;
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);


    // utworzenie okien na ka¿dym monitorze
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
    assert(screens.size() > 0);
    lastEventReceived = 0;
}
GUI::~GUI() {
    Shell_NotifyIcon(NIM_DELETE, &niData);
}
void GUI::ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        if (IsWindowVisible(hWnd))
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, L"Hide");
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, L"Show");
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, L"Exit");


        // note:	must set window to the foreground or the
        //			menu won't disappear when it should
        SetForegroundWindow(hWnd);

        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,
            pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}
GUI& GUI::GetInstance() {
    static GUI gui;
    return gui;
}
void GUI::MainLoop() {
    // Pêtla komunikatów
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
bool GUI::SetTrayIcon(int iconId, const wstring &info) {
    // This text will be shown as the icon's info.
    wsprintf(niData.szInfo, info.c_str());
    if (IDI_LASER_ICON <= iconId && iconId <= IDI_TCP) {
        niData.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconId),
            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR);
    }
    // Show the notification.;
    bool result = (Shell_NotifyIcon(NIM_MODIFY, &niData) == TRUE);
    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon))
        niData.hIcon = NULL;

    return result;
}
bool GUI::SetText(int textBoxId, const wstring &text) {
    HWND textBox = GetDlgItem(hWnd, textBoxId);
    return (SendMessage(textBox, WM_SETTEXT, 0, (LPARAM)text.c_str()) == TRUE);
}
void GUI::ProcRecvData(const Serwer *server, uint8_t *data, uint16_t dataLen)
{
    serverMutex.lock();
    if (connectedServers.size() > 0 && connectedServers.front() == server && dataLen >= 2) {
        uint16_t crc = CRC_INIT;
        for (uint16_t i = 0; i < dataLen; i++) 
            crc = CrcUpdate(crc, data[i]);
        if (crc == CRC_VALID) {
            clock_t eventReceived = clock();
            //przetwarzanie danych tylko z pierwszego serwera
            switch (data[0])
            {
            case msg_type_key:
                switch (data[1])
                {
                case msg_key_type_esc:
                    keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_f5:
                    keybd_event(VK_F5, MapVirtualKey(VK_F5, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_F5, MapVirtualKey(VK_F5, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_shf5:
                    keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_F5, MapVirtualKey(VK_F5, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_F5, MapVirtualKey(VK_F5, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_prev:
                    keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_next:
                    keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_left_down:
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    break;
                case msg_key_type_left_up:
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    break;
                case msg_key_type_right_down:
                    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                    break;
                case msg_key_type_right_up:
                    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                    break;
                case msg_key_type_volume_down:
                    keybd_event(VK_VOLUME_DOWN, MapVirtualKey(VK_VOLUME_DOWN, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_VOLUME_DOWN, MapVirtualKey(VK_VOLUME_DOWN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_volume_up:
                    keybd_event(VK_VOLUME_UP, MapVirtualKey(VK_VOLUME_UP, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_VOLUME_UP, MapVirtualKey(VK_VOLUME_UP, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                default:
                    break;
                }
                break;
            case msg_type_laser:
                if (data[1]) {
                    SetTrayIcon(IDI_LASER_ICON_ON, L"");
                    keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, MAPVK_VK_TO_VSC), 0, 0);
                    if ((eventReceived - lastEventReceived) < 5000) {
                        // bez zmiany pozycji
                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    }
                    else {
                        //wyœrodkowanie
                        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, 0x8000, 0x8000, 0, 0);
                    }
                }
                else {
                    SetTrayIcon(IDI_LASER_ICON_OFF, L"");
                    keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                }
                break;
            case msg_type_gesture:
                if (dataLen == 11) {
                    int ints[2], dx, dy;
                    memcpy(&ints[0], &data[1], 8);
                    dx = int(float(ints[0]) * screens[0].width / 1000000.0f);
                    dy = int(float(ints[1]) * screens[0].height / 1000000.0f);                    
                    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
                }
                break;
            case msg_type_gyro:
                if (dataLen == 11) {
                    int ints[2], dx, dy;
                    memcpy(&ints, &data[1], 8);
                    dx = int(float(ints[0]) / 1000000.0f * screens[0].width / float(M_PI / 3.0));
                    dy = int(float(ints[1]) / 1000000.0f * screens[0].width / float(M_PI / 3.0));
                    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
                }
                break;
            }
            lastEventReceived = eventReceived;
        }
    }
    serverMutex.unlock();
}
void GUI::Connected(const Serwer *server)
{
    serverMutex.lock();
    if (connectedServers.size() == 0) {
        SetTrayIcon(IDI_LASER_ICON_OFF, L"Remote client connected!");
    }
    if (find(connectedServers.begin(), connectedServers.end(), server) == connectedServers.end()) {
        connectedServers.push_back(server);
    }
    serverMutex.unlock();
}
void GUI::Disconnected(const Serwer *server)
{
    serverMutex.lock();
    auto server_it = find(connectedServers.begin(), connectedServers.end(), server);
    if (server_it != connectedServers.end()) {
        connectedServers.erase(server_it);
    }
    if (connectedServers.size() == 0) {
        SetTrayIcon(IDI_LASER_ICON, L"Remote client disconnected!");
    }
    serverMutex.unlock();
}