/**
 * \brief Nag��wek klasy interfejsu graficznego
 * \file GUI.hpp
 * \author Pawe� Iwaneczko
 */

#pragma once
#ifndef IG_GUI_H
#define IG_GUI_H

#include <Windows.h>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include "UpdateDownloader.hpp"

using namespace std;

#define REMOTE_SERVER_INSTANCE_STRING L"Remote Server"
// {f27b335a-e210-4aad-b890-8bc66ff26f2b}
static const GUID guidServiceClass = {0xf27b335a, 0xe210, 0x4aad, {0xb8, 0x90, 0x8b, 0xc6, 0x6f, 0xf2, 0x6f, 0x2b}};

/* Lista wyliczeniowa typu komunikat�w */
enum msg_type_t {
    msg_type_button = 0,   /*!< Przycisk    */
    msg_type_laser = 1,    /*!< Laser       */
    msg_type_gesture = 2,  /*!< Gest myszk� */
    msg_type_gyro = 3,     /*!< �yroskop    */
    msg_type_keyboard = 4, /*!< Klawisz     */
    msg_type_version = 5,  /*!< Wersja      */
    msg_type_wheel = 6     /*!< Rolka myszy */
};

enum server_type_en { stUnspecified, stBluetooth, stTCP };

/**
 * Klasa abstarackyjna serwera
 */
class Server {
protected:
    server_type_en serverType = stUnspecified;
    static uint16_t GetDataLen(msg_type_t type);

public:
    /**
     * Pobranie pola serverType
     * \return Pole serverType
     */
    server_type_en GetServerType() const;
};

/* Struktura prostok�tu monitora */
struct ScrrenRect {
    int left, top, width, height;
};

/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikon� systemow�
 */
class GUI {
private:
    // Update
    UpdateDownloader downloader;
    thread downloadThread;
    void DownloadThread();
    // Server Events
    clock_t lastEventReceived;
    int zoomCount;
    int ints[2], dx, dy;
    deque<const Server *> connectedServers;
    deque<ScrrenRect> screens;
    mutex serverMutex;
    HWND hWnd;
    bool hidden;
    HINSTANCE hInstance;
    NOTIFYICONDATA niData;
    friend LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    friend BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    void ShowContextMenu(HWND hWnd);
    // Konstruktor - tworzy okno dialogowe oraz ikon� systemow�
    GUI();
    // Destruktor - usuwa ikon� systemow�
    ~GUI();

public:
    /* Lista wyliczeniowa stau okna */
    enum window_state {
        wsDefault,  /**< Bez zmiany stanu okna */
        wsShow,     /**< Pokazanie okna        */
        wsHide,     /**< Ukrycie okna          */
        wsTimedHide /**< Ukrycie okna po 2s    */
    };
    /**
     * Pobieranie instancji klasy interfejsu graficznego aplikacji
     * \return Referencja na statyczny obiekt typu GUI
     */
    static GUI &GetInstance();
    /**
     * P�tla g��wna przetwarzaj�ca komunikaty systemowe. Ko�czy si� po zamkni�ciu aplikacji
     */
    void MainLoop();
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId     Identyfikator ikony. Warto�ci od 101 do 105
     * \param[in] info       Text informacyjny, kt�ry poka�e si� w powiadomieniu nad ikon� systemow�.
     *                       Aby nie wy�wietla� nale�y ustawi� ""
     * \param[in] iconFlag   Flaga ikony w powiadomieniu
     */
    bool SetTrayIcon(int iconId, const wstring &info = L"", int iconFlag = NIIF_NONE);
    /**
     * Ukrycie powiadomienia
     */
    bool HideTrayIcon();
    /**
     * Ustawienie textu pola tekstowego
     * \param[in] textBoxId Identyfikator pola tekstowego. Warto�ci od 1001 do 1006
     * \param[in] text Text pola tekstowego
     * \param[in] windowState Stan okna
     */
    bool SetText(int textBoxId, const wstring &text, window_state windowState = wsDefault);
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] server Wska�nik na serwer, kt�ry chce przetworzy� dane (Mo�e to zrobi� tylko jeden z dw�ch)
     * \param[in] data Wska�nik na bufor odebranych danych
     * \param[in] dataLen D�ugo�� danych
     */
    void ProcRecvData(const Server *server, uint8_t *data, uint16_t dataLen);
    /**
     * Po��czenie klienta do serwera
     * \param[in] server Wska�nik na serwer, do kt�rego po��czy� si� klient
     */
    void Connected(const Server *server);
    /**
     * Roz��czenie klienta z serwera
     * \param[in] server Wska�nik na serwer, z kt�rego ro��czy� si� klient
     */
    void Disconnected(const Server *server);
};

#endif /* IG_GUI_H */
