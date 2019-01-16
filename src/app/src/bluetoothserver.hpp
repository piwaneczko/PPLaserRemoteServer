/**
 * \brief Nag³ówek klasy serwera bluetooth
 * \file BluetoothServer.hpp
 * \author Pawe³ Iwaneczko
 */

#pragma once
#ifndef IG_BLUETOOTH_SERVER_H
#define IG_BLUETOOTH_SERVER_H

#include <thread>
#include "GUI.hpp"

/**
 * Klasa serwera Bluetooth
 */
class BluetoothServer : public Server {
    void mainLoop() override;

public:
    /**
     * Konstruktor klasy serwera Bluetooth
     * \param[in] gui Referencja na intefrejs graficzny
     */
    explicit BluetoothServer(Gui &gui);
    /* Destruktor klasy */
    ~BluetoothServer();
};

#endif /* IG_BLUETOOTH_SERVER_H */
