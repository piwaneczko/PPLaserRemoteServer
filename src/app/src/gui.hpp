/**
 * \brief Nag��wek klasy interfejsu graficznego
 * \file GUI.hpp
 * \author Pawe� Iwaneczko
 */

#pragma once
#ifndef IG_GUI_H
#define IG_GUI_H

#include <Windows.h>
#include <XmlConfig.hpp>
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

/* Lista wyliczeniowa typu komunikat�w */
enum msg_type_t {
    msg_type_button = 0,   /*!< Przycisk    */
    msg_type_laser = 1,    /*!< Laser       */
    msg_type_gesture = 2,  /*!< Gest myszk� */
    msg_type_gyro = 3,     /*!< �yroskop    */
    msg_type_keyboard = 4, /*!< Klawisz     */
    msg_type_volume = 5,   /*!< G�o�no��    */
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
    /**
     * Disconnecting client with shutdown (R&W)
     */
    void disconnect();
};

/* Struktura prostok�tu monitora */
struct ScrrenRect {
    int left, top, width, height;
};

wstring s2ws(const string &s);

/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikon� systemow�
 */
class Gui {
public:
    /* Lista wyliczeniowa stau okna */
    enum window_state {
        wsDefault,  /**< Bez zmiany stanu okna */
        wsShow,     /**< Pokazanie okna        */
        wsHide,     /**< Ukrycie okna          */
        wsTimedHide /**< Ukrycie okna po 2s    */
    };

private:
    // Update
    UpdateDownloader downloader;
    thread downloadThread;
    void downloadLoop();
    // Server Events
    clock_t lastEventReceived;
    int zoomCount;
    Server *connectedServer = nullptr;
    unique_ptr<Server> bluetoothServer;
    unique_ptr<Server> tcpServer;
    deque<ScrrenRect> screens;
    mutex serverMutex;
    HWND hWnd, settingsHwnd;
    bool hidden;
    HINSTANCE hInstance;
    NOTIFYICONDATA niData;
    static LRESULT CALLBACK dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK settingsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    friend BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    void showContextMenu(HWND hWnd) const;
    void initDialog(HWND hWnd) const;
    void saveSettings();
    void windowState(window_state state) const;

    // Move inertion
    bool moveLoopIsRunning;
    thread moveThread;
    deque<POINT> moveDeque;
    mutex moveLocker;
    void moveLoop();
    static void mouseEvent(uint16_t flag, int32_t dx = 0, int32_t dy = 0);

    // Volume control
    AudioVolume audioVolume;
    float volume_;
    deque<vector<uint8_t>> dataToSend;
    mutex sendLocker;
    void sendCurrentVolume();

    const string settingsFilePath;
    XmlConfigValue<bool, XmlConfigReadWriteFlag> showNotification;
    XmlConfigValue<bool, XmlConfigReadWriteFlag> soundNotification;
    XmlConfigValue<bool> tempDirectory;
    XmlConfigValue<bool, XmlConfigReadWriteFlag> useMoveThread;
    XmlConfigValue<uint32_t, XmlConfigReadWriteFlag> autoHide;

public:
    // Konstruktor - tworzy okno dialogowe oraz ikon� systemow�
    Gui();
    // Destruktor - usuwa ikon� systemow�
    ~Gui();
    /**
     * P�tla g��wna przetwarzaj�ca komunikaty systemowe. Ko�czy si� po zamkni�ciu aplikacji
     */
    void mainLoop() const;
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId     Identyfikator ikony. Warto�ci od 101 do 105
     * \param[in] info       Text informacyjny, kt�ry poka�e si� w powiadomieniu nad ikon� systemow�.
     *                       Aby nie wy�wietla� nale�y ustawi� ""
     * \param[in] iconFlag   Flaga ikony w powiadomieniu
     */
    bool setTrayIcon(int iconId, const wstring &info = L"", int iconFlag = NIIF_NONE);
    /**
     * Ukrycie powiadomienia
     */
    bool hideTrayIcon();
    /**
     * Ustawienie textu pola tekstowego
     * \param[in] textBoxId Identyfikator pola tekstowego. Warto�ci od 1001 do 1006
     * \param[in] text Text pola tekstowego
     * \param[in] windowState Stan okna
     */
    bool setText(int textBoxId, const wstring &text, window_state windowState = wsDefault) const;
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] server Wska�nik na serwer, kt�ry chce przetworzy� dane (Mo�e to zrobi� tylko jeden z dw�ch)
     * \param[in] data Wska�nik na bufor odebranych danych
     * \param[in] dataLen D�ugo�� danych
     */
    void procRecvData(const Server *server, uint8_t *data, uint16_t dataLen);
    /**
     * Po��czenie klienta do serwera
     * \param[in] server Wska�nik na serwer, do kt�rego po��czy� si� klient
     */
    void connected(Server *server);
    /**
     * Roz��czenie klienta z serwera
     * \param[in] server Wska�nik na serwer, z kt�rego ro��czy� si� klient
     */
    void disconnected(Server *server);
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
