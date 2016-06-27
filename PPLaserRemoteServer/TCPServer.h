/**
* \brief Nag³ówek klasy serwera TCP/IP
* \file TCPServer.h
* \author Pawe³ Iwaneczko
*/

#pragma once
#ifndef IG_TCP_SERVER_H
#define IG_TCP_SERVER_H

#include <thread>
#include "GUI.h"

/**
 * Klasa serwera TCP/IP
 */
class TCPServer : public Server
{
private:
    GUI &gui;
    thread listenThread;
    uint32_t listenSocket, clientSocket;
    bool listenThreadIsRunning;
    void ListenThread();
public:
    /**
     * Konstruktor klasy serwera TCP/IP
     * \param[in] gui Referencja na intefrejs graficzny
     */
    TCPServer(GUI &gui);
    /* Destruktor klasy */
    ~TCPServer();
};

#endif /* IG_TCP_SERVER_H */