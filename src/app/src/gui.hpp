/**
 * \brief Nag³ówek klasy interfejsu graficznego
 * \file GUI.hpp
 * \author Pawe³ Iwaneczko
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

/* Lista wyliczeniowa typu komunikatów */
enum msg_type_t {
    msg_type_button = 0,   /*!< Przycisk    */
    msg_type_laser = 1,    /*!< Laser       */
    msg_type_gesture = 2,  /*!< Gest myszk¹ */
    msg_type_gyro = 3,     /*!< ¯yroskop    */
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

/* Struktura prostok¹tu monitora */
struct ScrrenRect {
    int left, top, width, height;
};

/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikonê systemow¹
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
    // Konstruktor - tworzy okno dialogowe oraz ikonê systemow¹
    GUI();
    // Destruktor - usuwa ikonê systemow¹
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
     * Pêtla g³ówna przetwarzaj¹ca komunikaty systemowe. Koñczy siê po zamkniêciu aplikacji
     */
    void MainLoop();
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId     Identyfikator ikony. Wartoœci od 101 do 105
     * \param[in] info       Text informacyjny, który poka¿e siê w powiadomieniu nad ikon¹ systemow¹.
     *                       Aby nie wyœwietlaæ nale¿y ustawiæ ""
     * \param[in] iconFlag   Flaga ikony w powiadomieniu
     */
    bool SetTrayIcon(int iconId, const wstring &info = L"", int iconFlag = NIIF_NONE);
    /**
     * Ukrycie powiadomienia
     */
    bool HideTrayIcon();
    /**
     * Ustawienie textu pola tekstowego
     * \param[in] textBoxId Identyfikator pola tekstowego. Wartoœci od 1001 do 1006
     * \param[in] text Text pola tekstowego
     * \param[in] windowState Stan okna
     */
    bool SetText(int textBoxId, const wstring &text, window_state windowState = wsDefault);
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] server WskaŸnik na serwer, który chce przetworzyæ dane (Mo¿e to zrobiæ tylko jeden z dwóch)
     * \param[in] data WskaŸnik na bufor odebranych danych
     * \param[in] dataLen D³ugoœæ danych
     */
    void ProcRecvData(const Server *server, uint8_t *data, uint16_t dataLen);
    /**
     * Po³¹czenie klienta do serwera
     * \param[in] server WskaŸnik na serwer, do którego po³¹czy³ siê klient
     */
    void Connected(const Server *server);
    /**
     * Roz³¹czenie klienta z serwera
     * \param[in] server WskaŸnik na serwer, z którego ro³¹czy³ siê klient
     */
    void Disconnected(const Server *server);
};

#endif /* IG_GUI_H */
