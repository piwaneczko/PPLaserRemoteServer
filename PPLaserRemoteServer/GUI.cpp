/**
* \brief ¯ród³o klasy interfejsu graficznego
* \file GUI.cpp
* \author Pawe³ Iwaneczko
*/

#include "GUI.h"

using namespace std;

#define SWM_TRAYMSG	WM_APP          /**< The message ID sent to our window */
#define SWM_SHOW	WM_APP + 1      /**< Show the window                   */
#define SWM_HIDE	WM_APP + 2      /**< Hide the window                   */
#define SWM_EXIT	WM_APP + 3      /**< Close the window                  */

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
                ShowWindow(hWnd, SW_RESTORE);
                UpdateWindow(hWnd);
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

GUI::GUI() {
    hInstance = GetModuleHandle(0);
    hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc);

    HICON hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_LASER_ICON),
        IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

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
void GUI::ProcRecvData(uint8_t buff[], uint16_t dataLen)
{

}