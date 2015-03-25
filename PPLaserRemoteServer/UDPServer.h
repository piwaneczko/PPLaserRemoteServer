/**
* \brief Nag³ówek klasy serwera UDP/IP
* \file UDPServer.h
* \author Pawe³ Iwaneczko
*/

#pragma once
#ifndef IG_UDP_SERVER_H
#define IG_UDP_SERVER_H

#include <thread>
#include "GUI.h"

/**
 * Klasa serwera UDP/IP
 */
class UDPServer : public Serwer
{
private:
    GUI &gui;
    thread listenThread;
    uint32_t serverSocket;
    bool listenThreadIsRunning;
    void ListenThread();
public:
    /**
     * Konstruktor klasy serwera UDP/IP
     * \param[in] gui Referencja na intefrejs graficzny
     */
    UDPServer(GUI &gui);
    /* Destruktor klasy */
    ~UDPServer();
};

#endif /* IG_UDP_SERVER_H */
