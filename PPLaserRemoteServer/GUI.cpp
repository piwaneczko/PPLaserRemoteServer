/**
* \brief ¯ród³o klasy interfejsu graficznego
* \file GUI.cpp
* \author Pawe³ Iwaneczko
*/

#include "GUI.h"
#include <assert.h>
#include <math.h>
#include <ShObjIdl.h>
#include "XmlConfig.hpp"


using namespace std;

#define SWM_TRAYMSG	WM_APP          /**< The message ID sent to our window */
#define SWM_SHOW	WM_APP + 1      /**< Show the window                   */
#define SWM_HIDE	WM_APP + 2      /**< Hide the window                   */
#define SWM_EXIT	WM_APP + 3      /**< Close the window                  */
#define ID_TIMER 1

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
    msg_key_type_volume_up = 10,
    msg_key_type_zoom_in = 11,
    msg_key_type_zoom_out = 12
};

#define CRC_INIT		0xFFFFu
#define CRC_VALID		0xF0B8u


XmlConfigValue<bool> ShowNotification("ShowNotification", true);
XmlConfigValue<bool> SoundNotification("SoundNotification", false);
XmlConfigValue<bool> TempDirectory("UpdateTemporaryDirectory", true);
XmlConfigValue<uint32_t> AutoHide("Autohide time (milliseconds)", 3000);


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
        case WM_TIMER:
            KillTimer(hWnd, ID_TIMER);
            ShowWindow(hWnd, SW_HIDE);
            break;
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

GUI::GUI() : 
    downloader(
        L"https://docs.google.com/uc?authuser=0&id=0B-WV4mqjX8JgSUJkN0ptWWxSMmc&export=download",
        L"https://docs.google.com/uc?id=0B-WV4mqjX8JgQTVTcXNmWXRkaEE&export=download"),
    hidden()
{
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
    zoomCount = 1;
    downloadThread = thread(&GUI::DownloadThread, this);
}
GUI::~GUI() {
    Shell_NotifyIcon(NIM_DELETE, &niData);
    downloader.AbortDownload();
    if (downloadThread.joinable())
        downloadThread.join();
}
void ChangeUpdateGroupVisibility(HWND hWnd, bool visible) {
    //pokazanie kontrolek aktualizacji
    ShowWindow(GetDlgItem(hWnd, IDC_DOWNLOAD_TEXT), visible ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_DOWNLOAD_BAR), visible ? SW_SHOW : SW_HIDE);
    //uktycie innych kontrolek
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_GROUP), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_ICON), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_STATUS_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_SERVER_STATUS), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_S_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_SERVER_NAME), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_C_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_BTH_CLIENT_NAME), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_GROUP), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_ICON), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_STATUS_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_SERVER_STATUS), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_SIP_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_SERVER_IP), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_CIP_LABEL), visible ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(hWnd, IDC_TCP_CLIENT_IP), visible ? SW_HIDE : SW_SHOW);
}
void OnUpdateProgress(float value, void *user)
{
    HWND &hWnd = *(HWND *)user;
    HWND text = GetDlgItem(hWnd, IDC_DOWNLOAD_TEXT);
    HWND bar = GetDlgItem(hWnd, IDC_DOWNLOAD_BAR);
    if (text != NULL && bar != NULL) {
        int intValue = value * 100;
        SendMessage(text, WM_SETTEXT, 0, (LPARAM)(L"Downloading: " + to_wstring(intValue) + L"%").c_str());
        SendMessage(bar, PBM_SETPOS, (WPARAM)intValue, 0);
    }
}
void GUI::DownloadThread()
{
    wstring version = downloader.CheckVersion(UD_CHECK_BASE);
    if (!version.empty()) {
        KillTimer(hWnd, ID_TIMER);
        if (MessageBox(NULL,
            L"There is new version of software available to download.\nDo you want to install it now?",
            (L"New version (" + version + L") available!").c_str(),
            MB_YESNO | MB_ICONQUESTION) == IDYES) {
            ChangeUpdateGroupVisibility(hWnd, true);
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);
            SetTrayIcon(IDI_UPDATE, L"Downloading started!", NIIF_INFO);
            int result = downloader.DownloadAndUpdate(&OnUpdateProgress, (void *)&hWnd, TempDirectory);
            switch (result)
            {
            case S_OK:
                SetTrayIcon(IDI_UPDATE, L"Downloading ended!", NIIF_INFO);
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            case E_ABORT:
                SetTrayIcon(IDI_UPDATE, L"Downloading aborted!", NIIF_INFO);
                ChangeUpdateGroupVisibility(hWnd, false);
                UpdateWindow(hWnd);
                break;
            default:
                SetTrayIcon(IDI_UPDATE, L"Downloading error!", NIIF_ERROR);
                ChangeUpdateGroupVisibility(hWnd, false);
                UpdateWindow(hWnd);
                break;
            }
        }
    }
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
bool GUI::SetTrayIcon(int iconId, const wstring &info, int nifIconFlag) {    
    // This text will be shown as the icon's info.
    wsprintf(niData.szInfo, info.c_str());
    if (IDI_LASER_ICON <= iconId && iconId <= IDI_UPDATE) {
        niData.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconId),
            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
            LR_DEFAULTCOLOR);
    }
    niData.uFlags = NIF_ICON | (ShowNotification ? (NIF_TIP | NIF_INFO | NIF_MESSAGE) : 0);
    niData.dwInfoFlags = (SoundNotification ? 0 : NIIF_NOSOUND) | nifIconFlag;

    // Show the notification.;
    bool result = (Shell_NotifyIcon(hidden ? NIM_ADD : NIM_MODIFY, &niData) == TRUE);
    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon))
        niData.hIcon = NULL;

    hidden = !result;

    return result;
}
bool GUI::HideTrayIcon()
{
    hidden = (Shell_NotifyIcon(NIM_DELETE, &niData) == TRUE);
    return hidden;
}
bool GUI::SetText(int textBoxId, const wstring &text, window_state windowState) {
    HWND textBox = GetDlgItem(hWnd, textBoxId);
    int result = TRUE;
    switch (windowState)
    {
    case GUI::wsShow:
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        break;
    case GUI::wsHide:
        ShowWindow(hWnd, SW_HIDE);
        break;
    case GUI::wsTimedHide:
        if(AutoHide > 0u)
            result &= SetTimer(hWnd, ID_TIMER, AutoHide, NULL);
        break;
    default:
        break;
    }
    result &= SendMessage(textBox, WM_SETTEXT, 0, (LPARAM)text.c_str());
    return (result == TRUE);
}
string ws2s(const wstring& ws)
{
    int size_needed = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, ws.c_str(), (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

#if _DEBUG
size_t debugGoodCount = 0;
size_t debugBadCount = 0;
#endif
void keybd_event(uint8_t keyCode) {
    keybd_event(keyCode, MapVirtualKey(keyCode, MAPVK_VK_TO_VSC), 0, 0);
    keybd_event(keyCode, MapVirtualKey(keyCode, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
}
void GUI::ProcRecvData(const Server *server, uint8_t *data, uint16_t dataLen)
{
    serverMutex.lock();
    if (connectedServers.size() > 0 && connectedServers.front() == server && dataLen >= 2) {
        uint16_t crc = CRC_INIT;
        for (uint16_t i = 0; i < dataLen; i++) 
            crc = CrcUpdate(crc, data[i]);
#if _DEBUG
        if (crc == CRC_VALID)
            debugGoodCount++;
        else
            debugBadCount++;
        SetText(server->GetServerType() == stTCP ? IDC_TCP_SERVER_STATUS : IDC_BTH_SERVER_STATUS, L"Connected " + to_wstring(debugGoodCount) + L"/" + to_wstring(debugGoodCount + debugBadCount));
#endif
        if (crc == CRC_VALID) {
            clock_t eventReceived = clock();
            //przetwarzanie danych tylko z pierwszego serwera
            switch (data[0])
            {
            case msg_type_button:
                switch (data[1])
                {
                case msg_key_type_esc:
                    keybd_event(VK_ESCAPE);
                    break;
                case msg_key_type_f5:
                    keybd_event(VK_F5);
                    break;
                case msg_key_type_shf5:
                    keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_F5);
                    keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    break;
                case msg_key_type_prev:
                    keybd_event(VK_LEFT);
                    break;
                case msg_key_type_next:
                    keybd_event(VK_RIGHT);
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
                    keybd_event(VK_VOLUME_DOWN);
                    break;
                case msg_key_type_volume_up:
                    keybd_event(VK_VOLUME_UP);
                    break;
                case msg_key_type_zoom_in:
                    keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC), 0, 0);
                    keybd_event(VK_ADD);
                    keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
                    zoomCount++;
                    break;
                case msg_key_type_zoom_out:
                    keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC), 0, 0);
                    if (zoomCount > 1)
                        zoomCount--;
                    if (zoomCount > 1) {
                        keybd_event(VK_SUBTRACT);
                    }
                    else {
                        keybd_event(VK_ESCAPE);
                    }
                    keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
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
                    dx = int(float(ints[0]) * screens[0].width / (zoomCount * 1000000.0f));
                    dy = int(float(ints[1]) * screens[0].height / (zoomCount * 1000000.0f));
                    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
                }
                break;
            case msg_type_gyro:
                if (dataLen == 11) {
                    int ints[2], dx, dy;
                    memcpy(&ints, &data[1], 8);
                    dx = int(float(ints[0]) / (zoomCount * 1000000.0f) * screens[0].width / float(M_PI / 3.0));
                    dy = int(float(ints[1]) / (zoomCount * 1000000.0f) * screens[0].width / float(M_PI / 3.0));
                    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
                }
                break;
            case msg_type_keyboard:
                if (dataLen == 5) {
                    wchar_t keyCode = 0;
                    memcpy(&keyCode, &data[1], sizeof(wchar_t));
                    uint8_t byteKeyCode = (uint8_t)VkKeyScan(keyCode);
                    switch (byteKeyCode)
                    {
                    case VK_BACK: case VK_TAB: case VK_CLEAR: case VK_RETURN:
                    case VK_SHIFT: case VK_CONTROL: case VK_MENU: case VK_PAUSE: case VK_CAPITAL:
                    case VK_HANGUL: case VK_JUNJA: case VK_FINAL: case VK_HANJA:
                    case VK_ESCAPE: case VK_SPACE: case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME: 
                    case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
                    case VK_SELECT: case VK_PRINT: case VK_EXECUTE: case VK_SNAPSHOT:
                    case VK_INSERT: case VK_DELETE: case VK_HELP:
                    case VK_LWIN: case VK_RWIN: case VK_APPS: case VK_SLEEP:
                    case VK_F1: case VK_F2: case VK_F3: case VK_F4: case VK_F5: case VK_F6:
                    case VK_F7: case VK_F8: case VK_F9: case VK_F10: case VK_F11: case VK_F12:
                    case VK_F13: case VK_F14: case VK_F15: case VK_F16: case VK_F17: case VK_F18:
                    case VK_F19: case VK_F20: case VK_F21: case VK_F22: case VK_F23: case VK_F24:
                        keybd_event(byteKeyCode);
                        break;
                    default: {
                        INPUT ip;
                        ip.type = INPUT_KEYBOARD;
                        ip.ki.time = 0;
                        ip.ki.dwFlags = KEYEVENTF_UNICODE; // Specify the key as a unicode character
                        ip.ki.wScan = keyCode; // Which keypress to simulate
                        ip.ki.wVk = 0;
                        ip.ki.dwExtraInfo = 0;
                        SendInput(1, &ip, sizeof(INPUT));
                    } break;
                    }
                }
                break;
            }
            lastEventReceived = eventReceived;
        }
    }
    serverMutex.unlock();
}
void GUI::Connected(const Server *server)
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
void GUI::Disconnected(const Server *server)
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

uint16_t Server::GetDataLen(msg_type_t type) const
{
    uint16_t dlen = 0;
    switch (type)
    {
    case msg_type_gesture:
    case msg_type_gyro:
        dlen = 11;
        break;
    case msg_type_keyboard:
        dlen = 5;
        break;
    case msg_type_button:
    case msg_type_laser:
    default:
        dlen = 4;
        break;
    }
    return dlen;
}

server_type_en Server::GetServerType() const
{
    return this->serverType;
}
