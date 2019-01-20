/**
 * \brief ¯ród³o klasy interfejsu graficznego
 * \file GUI.cpp
 * \author Pawe³ Iwaneczko
 */

#include "GUI.hpp"
#include <ShObjIdl.h>
#include <assert.h>
#include <math.h>
#include <future>
#include "BluetoothServer.hpp"
#include "TCPServer.hpp"
#include "XmlConfig.hpp"
#include "resource.h"

using namespace std;

#define SWM_TRAYMSG WM_APP  /**< The message ID sent to our window */
#define SWM_SHOW WM_APP + 1 /**< Show the window                   */
#define SWM_HIDE WM_APP + 2 /**< Hide the window                   */
#define SWM_SETT WM_APP + 3 /**< Show settings                     */
#define SWM_EXIT WM_APP + 4 /**< Close the window                  */
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
    msg_key_type_zoom_out = 12,
    msg_key_type_pause = 13
};

#define CRC_INIT 0xFFFFu
#define CRC_VALID 0xF0B8u

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
static uint16_t CrcUpdate(uint16_t crc, uint8_t byte) {
    uint16_t h = uint8_t(crc ^ byte);
    h ^= uint8_t(uint16_t(h << 4));
    crc >>= 8;
    crc ^= uint16_t(h << 8);
    crc ^= uint16_t(h << 3);
    crc ^= uint16_t(h >> 4);
    return crc;
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (GetWindowLongPtr(hWnd, GWLP_USERDATA) != NULL) {
        auto &gui = *reinterpret_cast<Gui *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        switch (msg) {
            case SWM_TRAYMSG:
                switch (lParam) {
                    case WM_LBUTTONDBLCLK:
                        if (IsWindowVisible(hWnd)) {
                            ShowWindow(hWnd, SW_HIDE);
                        } else {
                            ShowWindow(hWnd, SW_RESTORE);
                            UpdateWindow(hWnd);
                        }
                        return 1;
                    case WM_RBUTTONDOWN:
                    case WM_CONTEXTMENU:
                        gui.showContextMenu(hWnd);
                    default:
                        break;
                }
                break;
            case WM_SYSCOMMAND:
                if ((wParam & 0xFFF0) == SC_MINIMIZE) {
                    ShowWindow(hWnd, SW_HIDE);
                    return 1;
                } else if ((wParam & 0xFFF0) == SC_CLOSE) {
                    DestroyWindow(hWnd);
                    return 1;
                }
                break;
            case WM_POWERBROADCAST:
                if (gui.connectedServer) gui.connectedServer->reset();
                return 1;
            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case SWM_SHOW:
                        ShowWindow(hWnd, SW_RESTORE);
                        break;
                    case SWM_HIDE:
                    case IDOK:
                        ShowWindow(hWnd, SW_HIDE);
                        break;
                    case SWM_SETT: {
                        wchar_t appFileName[MAX_PATH];
                        GetModuleFileName(nullptr, appFileName, MAX_PATH);
                        const auto xmlFilePath = wstring(appFileName) + L".xml";
                        ShellExecute(nullptr, L"open", xmlFilePath.c_str(), nullptr, nullptr, SW_SHOW);
                    } break;
                    case SWM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
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
    } else {
        // nie ustalono klasy GUI
        switch (msg) {
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
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    // uzyskanie wskaŸnika na glemm
    auto gui = reinterpret_cast<Gui *>(dwData);

    if (gui != nullptr) {
        const ScrrenRect rect = {lprcMonitor->left, lprcMonitor->top, lprcMonitor->right - lprcMonitor->left, lprcMonitor->bottom - lprcMonitor->top};
        gui->screens.push_back(rect);
    }

    return TRUE;
}

Gui::Gui()
    : downloader(L"https://github.com/piwaneczko/PPLaserRemoteServer/releases/download/update/ppremotesetup.info",
                 L"https://github.com/piwaneczko/PPLaserRemoteServer/releases/download/update/ppremotesetup.exe"),
      zoomCount(1),
      hidden(),
      audioVolume(this),
      volume_(audioVolume.volume()) {
    hInstance = GetModuleHandle(nullptr);
    hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), nullptr, DLGPROC(DlgProc));

    auto hIcon = HICON(LoadImage(hInstance, MAKEINTRESOURCE(IDI_LASER_ICON), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, LPARAM(hIcon));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, LPARAM(hIcon));

    if (hWnd == nullptr) exit(1);

    SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG(this));

    // Declare NOTIFYICONDATA details.
    // Error handling is omitted here for brevity. Do not omit it in your code.

    niData.cbSize = sizeof(niData);
    niData.hWnd = hWnd;
    niData.uVersion = NOTIFYICON_VERSION_4;
    niData.uFlags = NIF_ICON | NIF_TIP | NIF_INFO | NIF_MESSAGE;
    niData.hIcon = HICON(LoadImage(
        hInstance, MAKEINTRESOURCE(IDI_LASER_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    niData.uCallbackMessage = SWM_TRAYMSG;
    niData.szInfoTitle[0] = L'\0';
    niData.szInfo[0] = L'\0';

    // This text will be shown as the icon's tip.
    wsprintf(niData.szTip, REMOTE_SERVER_TITLE);

    // Show the notification.
    Shell_NotifyIcon(NIM_ADD, &niData);

    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon)) niData.hIcon = nullptr;
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    // utworzenie okien na ka¿dym monitorze
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, LPARAM(this));
    assert(screens.size() > 0);
    lastEventReceived = 0;
    downloadThread = thread(&Gui::downloadLoop, this);

    SetThreadExecutionState(ES_SYSTEM_REQUIRED);

    bluetoothServer = make_unique<BluetoothServer>(*this);
    tcpServer = make_unique<TCPServer>(*this);
    moveThread = thread(&Gui::moveLoop, this);
}
Gui::~Gui() {
    Shell_NotifyIcon(NIM_DELETE, &niData);
    downloader.AbortDownload();
    if (downloadThread.joinable()) downloadThread.join();
    moveLoopIsRunning = false;
    if (moveThread.joinable()) moveThread.join();
}
void changeUpdateGroupVisibility(HWND hWnd, bool visible) {
    // pokazanie kontrolek aktualizacji
    ShowWindow(GetDlgItem(hWnd, IDC_DOWNLOAD_TEXT), visible ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_DOWNLOAD_BAR), visible ? SW_SHOW : SW_HIDE);
    // uktycie innych kontrolek
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
void onUpdateProgress(float value, void *user) {
    auto &hWnd = *static_cast<HWND *>(user);
    const auto text = GetDlgItem(hWnd, IDC_DOWNLOAD_TEXT);
    const auto bar = GetDlgItem(hWnd, IDC_DOWNLOAD_BAR);
    if (text != nullptr && bar != nullptr) {
        const auto intValue = int(value * 100);
        SendMessage(text, WM_SETTEXT, 0, LPARAM((L"Downloading: " + to_wstring(intValue) + L"%").c_str()));
        SendMessage(bar, PBM_SETPOS, WPARAM(intValue), 0);
    }
}
void Gui::downloadLoop() {
    auto version = downloader.CheckVersion(UD_CHECK_BASE);
    if (!version.empty()) {
        KillTimer(hWnd, ID_TIMER);
        if (MessageBox(nullptr,
                       L"There is new version of software available to download.\nDo you want to install it now?",
                       (L"New version (" + version + L") available!").c_str(),
                       MB_YESNO | MB_ICONQUESTION)
            == IDYES) {
            changeUpdateGroupVisibility(hWnd, true);
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);
            setTrayIcon(IDI_UPDATE, L"Downloading started!", NIIF_INFO);
            const auto result = downloader.DownloadAndUpdate(&onUpdateProgress, static_cast<void *>(&hWnd), TempDirectory);
            switch (result) {
                case S_OK:
                    setTrayIcon(IDI_UPDATE, L"Downloading ended!", NIIF_INFO);
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
                case E_ABORT:
                    setTrayIcon(IDI_UPDATE, L"Downloading aborted!", NIIF_INFO);
                    changeUpdateGroupVisibility(hWnd, false);
                    UpdateWindow(hWnd);
                    break;
                default:
                    setTrayIcon(IDI_UPDATE, L"Downloading error!", NIIF_ERROR);
                    changeUpdateGroupVisibility(hWnd, false);
                    UpdateWindow(hWnd);
                    break;
            }
        }
    }
}

void Gui::showContextMenu(const HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    const auto hMenu = CreatePopupMenu();
    if (hMenu) {
        if (IsWindowVisible(hWnd))
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, L"Hide");
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, L"Show");
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SETT, L"Settings");
        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, L"Exit");

        // note:	must set window to the foreground or the
        //			menu won't disappear when it should
        SetForegroundWindow(hWnd);

        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, nullptr);
        DestroyMenu(hMenu);
    }
}

void Gui::moveLoop() {
    auto moveEnded = true;
    moveLoopIsRunning = true;
    POINT pv = {0, 0}, sp = {0, 0};
    while (moveLoopIsRunning) {
        if (!moveDeque.empty()) {
            if (moveEnded) {
                GetCursorPos(&pv);
                sp = pv;
            }
            lock_guard<mutex> lock(moveLocker);
            while (!moveDeque.empty()) {
                const auto mp = moveDeque.front();
                sp.x += mp.x;
                sp.y += mp.y;
                moveDeque.pop_front();
            }
        }
#define EP 0.5
        if (sp.x != pv.x || sp.y != pv.y) {
            const auto ppv = pv;
            pv.x = long(pv.x * (1.0 - EP) + sp.x * EP);
            pv.y = long(pv.y * (1.0 - EP) + sp.y * EP);
            mouseEvent(MOUSEEVENTF_MOVE, int(pv.x - ppv.x), int(pv.y - ppv.y));
            this_thread::sleep_for(5ms);
        } else if (moveDeque.empty()) {
            moveEnded = true;
            this_thread::sleep_for(100ms);
        }
    }
}

void Gui::mainLoop() {
    // Pêtla komunikatów
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
bool Gui::setTrayIcon(int iconId, const wstring &info, int nifIconFlag) {
    // This text will be shown as the icon's info.
    wsprintf(niData.szInfo, info.c_str());
    if (IDI_LASER_ICON <= iconId && iconId <= IDI_UPDATE) {
        niData.hIcon = HICON(
            LoadImage(hInstance, MAKEINTRESOURCE(iconId), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    }
    niData.uFlags = NIF_ICON | (ShowNotification ? (NIF_TIP | NIF_INFO | NIF_MESSAGE) : 0);
    niData.dwInfoFlags = (SoundNotification ? 0 : NIIF_NOSOUND) | nifIconFlag;

    // Show the notification.;
    const auto result = (Shell_NotifyIcon(hidden ? NIM_ADD : NIM_MODIFY, &niData) == TRUE);
    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon)) niData.hIcon = nullptr;

    hidden = !result;

    return result;
}
bool Gui::hideTrayIcon() {
    hidden = (Shell_NotifyIcon(NIM_DELETE, &niData) == TRUE);
    return hidden;
}
bool Gui::setText(int textBoxId, const wstring &text, window_state windowState) const {
    const auto textBox = GetDlgItem(hWnd, textBoxId);
    auto result = TRUE;
    switch (windowState) {
        case Gui::wsShow:
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            break;
        case Gui::wsHide:
            ShowWindow(hWnd, SW_HIDE);
            break;
        case Gui::wsTimedHide:
            if (AutoHide > 0u) result &= SetTimer(hWnd, ID_TIMER, AutoHide, nullptr);
            break;
        default:
            break;
    }
    result &= SendMessage(textBox, WM_SETTEXT, 0, LPARAM(text.c_str()));
    return (result == TRUE);
}

BOOL CALLBACK KillScreenSaverFunc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) PostMessage(hwnd, WM_CLOSE, 0, 0);
    return TRUE;
}
void KillScreenSaver() {
    BOOL isRunning;
    if (SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, NULL, &isRunning, NULL) && (isRunning == TRUE)) {
        const auto hdesk = OpenDesktop(TEXT("Screen-saver"), 0, FALSE, DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS);
        if (hdesk) {
            EnumDesktopWindows(hdesk, KillScreenSaverFunc, 0);
            CloseDesktop(hdesk);
        } else {
            PostMessage(GetForegroundWindow(), WM_CLOSE, 0, 0L);
        }
    }
}

#if _DEBUG
size_t debugGoodCount = 0;
size_t debugBadCount = 0;
#endif
enum KeyClickFlag { key_down, key_up, key_both };
void keybd_event(uint16_t keyCode, KeyClickFlag flag = key_both, uint8_t scancode = 0) {
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.dwExtraInfo = 0;
    ip.ki.time = 0;
    ip.ki.dwFlags = scancode == 2 ? KEYEVENTF_UNICODE : (((scancode == 1) && (flag == key_both)) ? KEYEVENTF_SCANCODE : 0);
    ip.ki.wVk = scancode == 2 ? 0 : keyCode;
    ip.ki.wScan = scancode == 2 ? keyCode : MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);

    if (flag != key_up) {
        SendInput(1, &ip, sizeof(INPUT));
    }

    if (flag != key_down) {
        ip.ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
        KillScreenSaver();
    }
}

void Gui::mouseEvent(uint16_t flag, int dx, int dy) {
    INPUT ip;
    ip.type = INPUT_MOUSE;
    ip.mi.dwExtraInfo = 0;
    if (flag == MOUSEEVENTF_WHEEL) {
        ip.mi.dx = ip.mi.dy = 0;
        if (dx != 0) {
            ip.mi.mouseData = dx * WHEEL_DELTA / 3;
            flag = MOUSEEVENTF_HWHEEL;
        } else
            ip.mi.mouseData = dy * WHEEL_DELTA / 3;
    } else {
        ip.mi.dx = dx;
        ip.mi.dy = dy;
        ip.mi.mouseData = 0;
    }
    ip.mi.time = 0;
    ip.mi.dwFlags = flag | (((flag & MOUSEEVENTF_ABSOLUTE) == 0) ? MOUSEEVENTF_VIRTUALDESK : 0);
    SendInput(1, &ip, sizeof(INPUT));
    KillScreenSaver();
}

void Gui::volumeChanged(float volume) {
    volume_ = volume;
    sendCurrentVolume();
}

bool Gui::popDataToSend(vector<uint8_t> &data) {
    lock_guard<mutex> lock(sendLocker);
    if (dataToSend.empty()) return false;

    data = dataToSend.front();
    dataToSend.pop_front();
    return true;
}

void Gui::sendCurrentVolume() {
    lock_guard<mutex> lock(sendLocker);
    dataToSend.emplace_back(vector<uint8_t>(2));
    auto &dataVec = dataToSend.back();
    dataVec[0] = uint8_t(msg_type_volume);
    dataVec[1] = uint8_t(volume_);
}

void Gui::procRecvData(const Server *server, uint8_t *data, uint16_t dataLen) {
    int ints[2], dx, dy;
    // serverMutex.lock();
    if (connectedServer == server && dataLen >= 2) {
        uint16_t crc = CRC_INIT;
        for (uint16_t i = 0; i < dataLen; i++) crc = CrcUpdate(crc, data[i]);
#if _DEBUG
        if (crc == CRC_VALID)
            debugGoodCount++;
        else
            debugBadCount++;
        setText(server->getServerType() == stTCP ? IDC_TCP_SERVER_STATUS : IDC_BTH_SERVER_STATUS,
                L"Connected " + to_wstring(debugGoodCount) + L"/" + to_wstring(debugGoodCount + debugBadCount));
#endif
        if (crc == CRC_VALID) {
            const auto eventReceived = clock();
            // przetwarzanie danych tylko z pierwszego serwera
            switch (data[0]) {
                case msg_type_button:
                    switch (data[1]) {
                        case msg_key_type_esc:
                            keybd_event(VK_ESCAPE);
                            break;
                        case msg_key_type_f5:
                            keybd_event(VK_F5);
                            break;
                        case msg_key_type_shf5:
                            keybd_event(VK_LSHIFT, key_down);
                            keybd_event(VK_F5);
                            keybd_event(VK_LSHIFT, key_up);
                            break;
                        case msg_key_type_prev:
                            keybd_event(VK_LEFT);
                            break;
                        case msg_key_type_next:
                            keybd_event(VK_RIGHT);
                            break;
                        case msg_key_type_left_down:
                            mouseEvent(MOUSEEVENTF_LEFTDOWN);
                            break;
                        case msg_key_type_left_up:
                            mouseEvent(MOUSEEVENTF_LEFTUP);
                            break;
                        case msg_key_type_right_down:
                            mouseEvent(MOUSEEVENTF_RIGHTDOWN);
                            break;
                        case msg_key_type_right_up:
                            mouseEvent(MOUSEEVENTF_RIGHTUP);
                            break;
                        case msg_key_type_volume_down: {
                            keybd_event(VK_VOLUME_DOWN);
                            if (volume_ < 1.0f) sendCurrentVolume();
                        } break;
                        case msg_key_type_volume_up: {
                            keybd_event(VK_VOLUME_UP);
                            if (volume_ > 99.0f) sendCurrentVolume();
                        } break;
                        case msg_key_type_zoom_in:
                            if (zoomCount == 1) {
                                // duplicate
                                long i = 0;
                                DISPLAY_DEVICE dd;
                                DEVMODE dm;
                                ZeroMemory(&dd, sizeof(DISPLAY_DEVICE));
                                dd.cb = sizeof(DISPLAY_DEVICE);
                                while (EnumDisplayDevices(nullptr, i, &dd, 0)) {
                                    ZeroMemory(&dm, sizeof(DEVMODE));
                                    dm.dmSize = sizeof(DEVMODE);
                                    if (EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
                                        dm.dmPosition.x = dm.dmPosition.y = 0;
                                        ChangeDisplaySettingsEx(dd.DeviceName, &dm, nullptr, 0, nullptr);
                                    }
                                    i++;
                                    ZeroMemory(&dd, sizeof(DISPLAY_DEVICE));
                                    dd.cb = sizeof(DISPLAY_DEVICE);
                                }
                            }
                            keybd_event(VK_LWIN, key_down);
                            keybd_event(VK_ADD);
                            keybd_event(VK_LWIN, key_up);
                            zoomCount++;
                            break;
                        case msg_key_type_zoom_out:
                            if (zoomCount > 1) {
                                keybd_event(VK_LWIN, key_down);
                                keybd_event(zoomCount == 2 ? VK_ESCAPE : VK_SUBTRACT);
                                keybd_event(VK_LWIN, key_up);
                                zoomCount--;
                                if (zoomCount == 1) {
                                    // poprzedni
                                }
                            }
                            break;
                        case msg_key_type_pause:
                            keybd_event(BYTE(VkKeyScan('B')), 0xB0, 0, 0);
                            break;
                        default:
                            break;
                    }
                    break;
                case msg_type_laser:
                    if (data[1]) {
                        setTrayIcon(IDI_LASER_ICON_ON, L"");
                        keybd_event(VK_LCONTROL, key_down);
                        if ((eventReceived - lastEventReceived) < 5000) {
                            // bez zmiany pozycji
                            mouseEvent(MOUSEEVENTF_LEFTDOWN);
                        } else {
                            // wyœrodkowanie
                            mouseEvent(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, 0x8000, 0x8000);
                        }
                    } else {
                        setTrayIcon(IDI_LASER_ICON_OFF, L"");
                        keybd_event(VK_LCONTROL, key_up);
                        mouseEvent(MOUSEEVENTF_LEFTUP);
                    }
                    break;
                case msg_type_gesture:
                    if (dataLen == 11) {
                        memcpy(&ints[0], &data[1], 8);
                        dx = int(float(ints[0]) * screens[0].width / (zoomCount * 1000000.0f));
                        dy = int(float(ints[1]) * screens[0].height / (zoomCount * 1000000.0f));
                        // mouseEvent(MOUSEEVENTF_MOVE, dx, dy);
                        lock_guard<mutex> lock(moveLocker);
                        moveDeque.push_back(POINT{dx, dy});
                    }
                    break;
                case msg_type_wheel:
                    if (dataLen == 11) {
                        memcpy(&ints[0], &data[1], 8);
                        dx = ints[0];
                        dy = ints[1];
                        mouseEvent(MOUSEEVENTF_WHEEL, dx, dy);
                    }
                    break;
                case msg_type_gyro:
                    if (dataLen == 11) {
                        memcpy(&ints, &data[1], 8);
                        dx = int(float(ints[0]) / (zoomCount * 1000000.0f) * screens[0].width / float(M_PI / 3.0));
                        dy = int(float(ints[1]) / (zoomCount * 1000000.0f) * screens[0].width / float(M_PI / 3.0));
                        mouseEvent(MOUSEEVENTF_MOVE, dx, dy);
                        lock_guard<mutex> lock(moveLocker);
                        moveDeque.push_back(POINT{dx, dy});
                    }
                    break;
                case msg_type_keyboard:
                    if (dataLen == 5) {
                        wchar_t keyCode = 0;
                        memcpy(&keyCode, &data[1], sizeof(wchar_t));
                        const auto byteKeyCode = uint8_t(VkKeyScan(keyCode));
                        switch (byteKeyCode) {
                            case VK_BACK:
                            case VK_TAB:
                            case VK_CLEAR:
                            case VK_RETURN:
                            case VK_SHIFT:
                            case VK_CONTROL:
                            case VK_MENU:
                            case VK_PAUSE:
                            case VK_CAPITAL:
                            case VK_HANGUL:
                            case VK_JUNJA:
                            case VK_FINAL:
                            case VK_HANJA:
                            case VK_ESCAPE:
                            case VK_SPACE:
                            case VK_PRIOR:
                            case VK_NEXT:
                            case VK_END:
                            case VK_HOME:
                            case VK_LEFT:
                            case VK_UP:
                            case VK_RIGHT:
                            case VK_DOWN:
                            case VK_SELECT:
                            case VK_PRINT:
                            case VK_EXECUTE:
                            case VK_SNAPSHOT:
                            case VK_INSERT:
                            case VK_DELETE:
                            case VK_HELP:
                            case VK_LWIN:
                            case VK_RWIN:
                            case VK_APPS:
                            case VK_SLEEP:
                            case VK_F1:
                            case VK_F2:
                            case VK_F3:
                            case VK_F4:
                            case VK_F5:
                            case VK_F6:
                            case VK_F7:
                            case VK_F8:
                            case VK_F9:
                            case VK_F10:
                            case VK_F11:
                            case VK_F12:
                            case VK_F13:
                            case VK_F14:
                            case VK_F15:
                            case VK_F16:
                            case VK_F17:
                            case VK_F18:
                            case VK_F19:
                            case VK_F20:
                            case VK_F21:
                            case VK_F22:
                            case VK_F23:
                            case VK_F24:
                                keybd_event(byteKeyCode, key_both, 1);
                                break;
                            default:
                                keybd_event(keyCode, key_down, 2);
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            lastEventReceived = eventReceived;
        }
    }
    // serverMutex.unlock();
}
void Gui::connected(Server *server) {
    lock_guard<mutex> lock(serverMutex);
    if (connectedServer != server) {
        if (connectedServer != nullptr) connectedServer->disconnect();
        setTrayIcon(IDI_LASER_ICON_OFF, L"Remote client connected!");
        connectedServer = server;
    }
}
void Gui::disconnected(Server *server) {
    unique_lock<mutex> lock(serverMutex, try_to_lock);
    if (server == connectedServer) {
        connectedServer = nullptr;
        setTrayIcon(IDI_LASER_ICON, L"Remote client disconnected!");
    }
}

Server::Server(Gui &gui) : gui(gui) {}

uint16_t Server::getDataLen(msg_type_t type) {
    uint16_t dlen;
    switch (type) {
        case msg_type_gesture:
        case msg_type_wheel:
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

server_type_en Server::getServerType() const {
    return this->serverType;
}

#define SD_BOTH 0x02
void Server::disconnect() {
    if (INVALID_SOCKET != clientSocket) {
        shutdown(clientSocket, SD_BOTH);
        clientSocket = INVALID_SOCKET;
    }
}

void Server::resetLoop() {
    destroy();
    init();
    resetThread.detach();
}

void Server::reset() {
    resetThread = thread(&Server::resetLoop, this);
}

wstring s2ws(const string &s) {
    const auto sizeNeeded = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    std::wstring strTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &strTo[0], sizeNeeded);
    return strTo;
}
