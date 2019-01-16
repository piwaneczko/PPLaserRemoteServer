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
#include <vector>
#include "UpdateDownloader.hpp"
#include "audiovolume.hpp"

using namespace std;

#define REMOTE_SERVER_TITLE L"PP Remote Server"
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
    msg_type_volume = 5,   /*!< G³oœnoœæ    */
    msg_type_wheel = 6     /*!< Rolka myszy */
};

enum server_type_en { stUnspecified, stBluetooth, stTCP };

/**
 * Klasa abstarackyjna serwera
 */
class Server {
protected:
    Gui &gui;
    thread listenThread;
    uint32_t listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
    bool mainLoopIsRunning = false;
    virtual void mainLoop() = 0;

    server_type_en serverType = stUnspecified;
    static uint16_t getDataLen(msg_type_t type);

public:
    explicit Server(Gui &gui);
    virtual ~Server() = default;
    /**
     * Pobranie pola serverType
     * \return Pole serverType
     */
    server_type_en getServerType() const;
};

/* Struktura prostok¹tu monitora */
struct ScrrenRect {
    int left, top, width, height;
};

wstring s2ws(const string &s);

/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikonê systemow¹
 */
class Gui {
private:
    // Update
    UpdateDownloader downloader;
    thread downloadThread;
    void downloadLoop();
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
    static void showContextMenu(HWND hWnd);
    // Konstruktor - tworzy okno dialogowe oraz ikonê systemow¹
    Gui();
    // Destruktor - usuwa ikonê systemow¹
    ~Gui();

    // Volume control
    AudioVolume audioVolume;
    float volume_;
    deque<vector<uint8_t>> dataToSend;
    mutex sendLocker;
    void sendCurrentVolume();

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
    static Gui &getInstance();
    /**
     * Pêtla g³ówna przetwarzaj¹ca komunikaty systemowe. Koñczy siê po zamkniêciu aplikacji
     */
    static void mainLoop();
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId     Identyfikator ikony. Wartoœci od 101 do 105
     * \param[in] info       Text informacyjny, który poka¿e siê w powiadomieniu nad ikon¹ systemow¹.
     *                       Aby nie wyœwietlaæ nale¿y ustawiæ ""
     * \param[in] iconFlag   Flaga ikony w powiadomieniu
     */
    bool setTrayIcon(int iconId, const wstring &info = L"", int iconFlag = NIIF_NONE);
    /**
     * Ukrycie powiadomienia
     */
    bool hideTrayIcon();
    /**
     * Ustawienie textu pola tekstowego
     * \param[in] textBoxId Identyfikator pola tekstowego. Wartoœci od 1001 do 1006
     * \param[in] text Text pola tekstowego
     * \param[in] windowState Stan okna
     */
    bool setText(int textBoxId, const wstring &text, window_state windowState = wsDefault) const;
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] server WskaŸnik na serwer, który chce przetworzyæ dane (Mo¿e to zrobiæ tylko jeden z dwóch)
     * \param[in] data WskaŸnik na bufor odebranych danych
     * \param[in] dataLen D³ugoœæ danych
     */
    void procRecvData(const Server *server, uint8_t *data, uint16_t dataLen);
    /**
     * Po³¹czenie klienta do serwera
     * \param[in] server WskaŸnik na serwer, do którego po³¹czy³ siê klient
     */
    void connected(const Server *server);
    /**
     * Roz³¹czenie klienta z serwera
     * \param[in] server WskaŸnik na serwer, z którego ro³¹czy³ siê klient
     */
    void disconnected(const Server *server);
    /**
     * \brief Volume changed event occured
     * \param volume Audio Volume 0-100
     */
    void volumeChanged(float volume);
    /**
     * \brief Get and pop data to send from waiting deque
     * \param data Reference to data to send
     * \return Is something to send
     */
    bool popDataToSend(vector<uint8_t> &data);
};

#endif /* IG_GUI_H */
