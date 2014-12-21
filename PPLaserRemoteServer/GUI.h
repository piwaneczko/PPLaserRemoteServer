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
#include <Windows.h>
#include "resource.h"

using namespace std;

#define REMOTE_SERVER_INSTANCE_STRING     L"Remote Server"
#define DEFAULT_LISTEN_BACKLOG            4
#define RECV_BUFF_MAX_LEN                 1024      /**< Maksymalny rozmiar bufora odebranych danych */
// {F27B335A-E210-4AAD-B890-8BC66FF26F2B}
static const GUID guidServiceClass = 
{ 0xf27b335a, 0xe210, 0x4aad, { 0xb8, 0x90, 0x8b, 0xc6, 0x6f, 0xf2, 0x6f, 0x2b } };


/**
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikon� systemow�
 */
class GUI {
private:
    HWND hWnd;
    HINSTANCE hInstance;
    NOTIFYICONDATA niData;
    friend LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void ShowContextMenu(HWND hWnd);
    // Konstruktor - tworzy okno dialogowe oraz ikon� systemow�
    GUI();
    //Destruktor - usuwa ikon� systemow�
    ~GUI();
public:
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
     */
    bool SetText(int textBoxId, const wstring &text);
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] data Wska�nik na bufor odebranych danych
     * \param[in] dataLen D�ugo�� danych
     */
    void ProcRecvData(uint8_t buff[], uint16_t dataLen);
};

#endif /* IG_GUI_H */