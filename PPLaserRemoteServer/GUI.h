/**
* \brief Nag��wek klasy interfejsu graficznego
* \file GUI.h
* \author Pawe� Iwaneczko
*/

#pragma once
#ifndef IG_GUI_H
#define IG_GUI_H

#include <iostream>
#include <cstdint>
#include <string>
#include <deque>
#include <mutex>
#include <Windows.h>
#include "resource.h"

using namespace std;

#define REMOTE_SERVER_INSTANCE_STRING     L"Remote Server"
#define DEFAULT_LISTEN_BACKLOG            4
// {f27b335a-e210-4aad-b890-8bc66ff26f2b}
static const GUID guidServiceClass = 
{ 0xf27b335a, 0xe210, 0x4aad, { 0xb8, 0x90, 0x8b, 0xc6, 0x6f, 0xf2, 0x6f, 0x2b } };

/* Lista wyliczeniowa typu komunikat�w */
enum msg_type_t {
    msg_type_key = 0,               /**< Klawisz     */
    msg_type_laser = 1,             /**< Laser       */
    msg_type_gesture = 2,           /**< Gest myszk� */
    msg_type_gyro = 3               /**< �yroskop    */
};

/**
 * Klasa abstarackyjna serwera
 */

class Serwer {};

/* Struktura prostok�tu monitora */
struct ScrrenRect {
    int left, top, width, height;
};

/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikon� systemow�
 */
class GUI {
private:
    clock_t lastEventReceived;
    int zoomCount;
    deque<const Serwer *> connectedServers;
    deque<ScrrenRect> screens;
    mutex serverMutex;
    HWND hWnd;
    HINSTANCE hInstance;
    NOTIFYICONDATA niData;
    friend LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    friend BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    void ShowContextMenu(HWND hWnd);
    // Konstruktor - tworzy okno dialogowe oraz ikon� systemow�
    GUI();
    //Destruktor - usuwa ikon� systemow�
    ~GUI();
public:
    /* Lista wyliczeniowa stau okna */
    enum window_state
    {
        wsDefault,              /**< Bez zmiany stanu okna */
        wsShow,                 /**< Pokazanie okna        */
        wsHide,                 /**< Ukrycie okna          */
        wsTimedHide             /**< Ukrycie okna po 2s    */
    };
    /**
     * Pobieranie instancji klasy interfejsu graficznego aplikacji
     * \return Referencja na statyczny obiekt typu GUI
     */
    static GUI& GetInstance();
    /**
     * P�tla g��wna przetwarzaj�ca komunikaty systemowe. Ko�czy si� po zamkni�ciu aplikacji
     */
    void MainLoop();
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId Identyfikator ikony. Warto�ci od 101 do 105
     * \param[in] info Text informacyjny, kt�ry poka�e si� w powiadomieniu nad ikon� systemow�. 
     *                Aby nie wy�wietla� nale�y ustawi� ""
     */
    bool SetTrayIcon(int iconId, const wstring &info = L"");
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
    void ProcRecvData(const Serwer *server, uint8_t *buff, uint16_t dataLen);
    /**
     * Po��czenie klienta do serwera
     * \param[in] server Wska�nik na serwer, do kt�rego po��czy� si� klient
     */
    void Connected(const Serwer *server);
    /**
     * Roz��czenie klienta z serwera
     * \param[in] server Wska�nik na serwer, z kt�rego ro��czy� si� klient
     */
    void Disconnected(const Serwer *server);
};

#endif /* IG_GUI_H */