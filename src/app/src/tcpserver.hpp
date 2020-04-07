/**
 * \brief Nag³ówek klasy serwera TCP/IP
 * \file TCPServer.hpp
 * \author Pawe³ Iwaneczko
 */

#pragma once
#ifndef IG_TCP_SERVER_H
#define IG_TCP_SERVER_H

#include <XmlConfig.hpp>
#include "GUI.hpp"

/**
 * Klasa serwera TCP/IP
 */
class TCPServer : public Server {
    uint16_t port;
    XmlConfigValue<uint16_t, XmlConfigReadWriteFlag> defaultPort;
    void mainLoop() override;
    void setServerIPs(uint16_t port);

public:
    /**
     * Konstruktor klasy serwera TCP/IP
     * \param[in] gui Referencja na intefrejs graficzny
     */
    explicit TCPServer(Gui &gui);
    /* Destruktor klasy */
    ~TCPServer();
};

#endif /* IG_TCP_SERVER_H */
