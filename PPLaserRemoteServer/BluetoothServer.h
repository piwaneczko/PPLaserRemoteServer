/**
* \brief Nag³ówek klasy serwera bluetooth
* \file BluetoothServer.h
* \author Pawe³ Iwaneczko
*/

#pragma once
#ifndef IG_BLUETOOTH_SERVER_H
#define IG_BLUETOOTH_SERVER_H

#include <thread>
#include "GUI.h"

/**
 * Klasa serwera Bluetooth
 */
class BluetoothServer : public Serwer
{
private:
    GUI &gui;
    thread listenThread;
    uint32_t listenSocket, clientSocket;
    bool listenThreadIsRunning;
    void ListenThread();
public:
    /**
     * Konstruktor klasy serwera Bluetooth
     * \param[in] gui Referencja na intefrejs graficzny
     */
    BluetoothServer(GUI &gui);
    /* Destruktor klasy */
    ~BluetoothServer();
};

#endif /* IG_BLUETOOTH_SERVER_H */