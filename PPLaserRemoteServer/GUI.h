/**
* \brief Nag³ówek klasy interfejsu graficznego
* \file GUI.h
* \author Pawe³ Iwaneczko
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
 * Klasa interfejsu graficznego. Klasa tworzy okno dialogowe oraz ikonê systemow¹
 */
class GUI {
private:
    HWND hWnd;
    HINSTANCE hInstance;
    NOTIFYICONDATA niData;
    friend LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void ShowContextMenu(HWND hWnd);
    // Konstruktor - tworzy okno dialogowe oraz ikonê systemow¹
    GUI();
    //Destruktor - usuwa ikonê systemow¹
    ~GUI();
public:
    /**
     * Pobieranie instancji klasy interfejsu graficznego aplikacji
     * \return Referencja na statyczny obiekt typu GUI
     */
    static GUI& GetInstance();
    /**
     * Pêtla g³ówna przetwarzaj¹ca komunikaty systemowe. Koñczy siê po zamkniêciu aplikacji
     */
    void MainLoop();
    /**
     * Modyfikacja obrazu oraz tekstu informacyjnego ikony w pasku systemowym
     * \param[in] iconId Identyfikator ikony. Wartoœci od 101 do 105
     * \param[in] info Text informacyjny, który poka¿e siê w powiadomieniu nad ikon¹ systemow¹. 
     *                Aby nie wyœwietlaæ nale¿y ustawiæ ""
     */
    bool SetTrayIcon(int iconId, const wstring &info = L"");
    /**
     * Ustawienie textu pola tekstowego
     * \param[in] textBoxId Identyfikator pola tekstowego. Wartoœci od 1001 do 1006
     * \param[in] text Text pola tekstowego
     */
    bool SetText(int textBoxId, const wstring &text);
    /**
     * Przetworzenie danych od zdalnego klienta
     * \param[in] data WskaŸnik na bufor odebranych danych
     * \param[in] dataLen D³ugoœæ danych
     */
    void ProcRecvData(uint8_t buff[], uint16_t dataLen);
};

#endif /* IG_GUI_H */