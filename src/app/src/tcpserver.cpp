/**
 * \brief èrÛd≥o klasy serwera Bluetooth
 * \file TCPServer.cpp
 * \author Pawe≥ Iwaneczko
 */
#include <WinSock2.h>
#include <stdio.h>
#include <ws2tcpip.h>
// TCPServer.hpp include need to be agter ws2tcpip
#include "TCPServer.hpp"
#include "XmlConfig.hpp"
#include "resource.h"

using namespace std;

#define RECV_BUFF_MAX_LEN 22 /**< Maksymalny rozmiar bufora odebranych danych */

XmlConfigValue<uint16_t, XmlConfigReadWriteFlag> DefaultPort("TcpDefaultPort", 3389);

bool checkPortTcp(uint16_t port) {
    sockaddr_in client;
    auto sock = INVALID_SOCKET;
    auto result = false;

    memset(static_cast<void *>(&client), 0, sizeof(client));  // czyszczenie pamiÍci
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = int(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));

    if (sock == INVALID_SOCKET) {  // nie udala sie inicjalizacja socketa
        printf("socket failed with error: %ld\n", WSAGetLastError());
    }

    if (connect(sock, reinterpret_cast<sockaddr *>(&client), sizeof(client)) == SOCKET_ERROR) {
        const auto iResult = WSAGetLastError();
        if (iResult == WSAECONNREFUSED) {
            result = true;
        } else {
            throw exception(("connect to port " + to_string(port) + " failed with error: " + to_string(iResult)).c_str());
        }
    }

    closesocket(sock);

    return result;
}

TCPServer::TCPServer(Gui &gui) : Server(gui) {
    serverType = stTCP;

    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        gui.setText(IDC_TCP_SERVER_STATUS, L"WSAStartup error!", Gui::wsShow);
    }

    TCPServer::init();
}
TCPServer::~TCPServer() {
    TCPServer::destroy();
}

void TCPServer::mainLoop() {
    uint16_t offs = 0, len = 0, dlen = 0;
    uint8_t buff[RECV_BUFF_MAX_LEN] = {0};
    auto recvResult = 0;
    clientSocket = INVALID_SOCKET;
    sockaddr_in clientAddr = {0};
    int clientAddrLen = sizeof(sockaddr_in);

    FD_SET fdrecv;
    timeval timeout = {0, 1000};  // 1ms

    if (DefaultPort() != port) {
        if (MessageBox(nullptr,
                       (L"TCP port " + to_wstring(DefaultPort())
                        + L" is used by other program.\n"
                          L"Do you want to store currently TCP port "
                        + to_wstring(port) + L" in settings?")
                           .c_str(),
                       L"TCP port is used by other program",
                       MB_ICONQUESTION | MB_YESNO)
            == IDYES)
            DefaultPort = port;
    }

    gui.setText(IDC_TCP_CLIENT_IP, L"None", Gui::wsTimedHide);
    mainLoopIsRunning = true;
    while (mainLoopIsRunning) {
        setServerIPs(port);
        gui.setText(IDC_TCP_CLIENT_IP, L"None");
        gui.setText(IDC_TCP_SERVER_STATUS, L"Listening...");

        clientAddr = {0};
        clientAddrLen = sizeof(sockaddr_in);
        ZeroMemory(&clientAddr, sizeof(clientAddr));
        clientSocket = accept(listenSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (mainLoopIsRunning) {
                gui.setText(IDC_TCP_SERVER_STATUS, L"accept Error!", Gui::wsShow);
            } else {
                gui.setText(IDC_TCP_SERVER_STATUS, L"Ending listening", Gui::wsShow);
            }
            break;  // Break out of the for loop
        }

        gui.setText(IDC_TCP_SERVER_STATUS, L"Connected");

        char ipBuf[INET_ADDRSTRLEN];
        string clientIP = inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
        gui.setText(IDC_TCP_CLIENT_IP, wstring(clientIP.begin(), clientIP.end()));
        gui.connected(this);

        while (mainLoopIsRunning) {
            FD_ZERO(&fdrecv);
            FD_SET(clientSocket, &fdrecv);
            const auto numReady = select(0, &fdrecv, nullptr, nullptr, &timeout);
            if (!mainLoopIsRunning)
                break;
            else if (numReady < 0) {
                gui.setText(IDC_TCP_SERVER_STATUS, L"recv Error!", Gui::wsShow);
                break;
            } else if (numReady > 0 && FD_ISSET(clientSocket, &fdrecv) != 0)  // jest cos do odczytania
            {
                offs = 0;
                if (len >= RECV_BUFF_MAX_LEN) len = 0;
                recvResult = recv(clientSocket, reinterpret_cast<char *>(buff + len), RECV_BUFF_MAX_LEN - len, 0);
                if (recvResult == 0) {  // socket connection has been closed gracefully
                    gui.setText(IDC_TCP_SERVER_STATUS, L"Device connection was lost!");
                    break;  // Break out of the for loop
                } else if (recvResult == SOCKET_ERROR) {
                    gui.setText(IDC_TCP_SERVER_STATUS, L"recv Error!", Gui::wsShow);
                    break;  // Break out of the for loop
                } else {
                    len += recvResult;
                    // odebrano dane - dzielenie przetwarzanie
                    while (len >= 4) {
                        dlen = getDataLen((msg_type_t)buff[offs]);

                        if (dlen > len) break;

                        gui.procRecvData(this, &buff[offs], dlen);
                        offs += dlen;
                        len -= dlen;
                    }
                    if (len > 0) {
                        memcpy(&buff[0], &buff[offs], len);
                    }
                }
            } else {
                vector<uint8_t> data;
                while (gui.popDataToSend(data)) {
                    send(clientSocket, reinterpret_cast<const char *>(data.data()), data.size(), 0);
                }
            }
        }
        gui.disconnected(this);
        if (INVALID_SOCKET != clientSocket) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
    }

    if (INVALID_SOCKET != listenSocket) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    mainLoopIsRunning = false;
}

void TCPServer::init() {
    gui.setText(IDC_TCP_SERVER_STATUS, L"Initialization...");

    uint16_t port = DefaultPort;
    try {
        while (!checkPortTcp(port)) port++;
    } catch (const exception &e) {
        gui.setText(IDC_TCP_SERVER_STATUS, s2ws(e.what()).c_str(), Gui::wsShow);
    }
    this->port = port;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        gui.setText(IDC_TCP_SERVER_STATUS, L"socket Error!", Gui::wsShow);
    } else {
        sockaddr_in sockAddrTcpLocal = {0};
        const auto sockAddrTcpLocalLen = sizeof(sockaddr_in);
        sockAddrTcpLocal.sin_family = AF_INET;
        sockAddrTcpLocal.sin_addr.s_addr = inet_addr("0.0.0.0");
        sockAddrTcpLocal.sin_port = htons(port);

        if (SOCKET_ERROR == ::bind(listenSocket, reinterpret_cast<sockaddr *>(&sockAddrTcpLocal), sockAddrTcpLocalLen)) {
            gui.setText(IDC_TCP_SERVER_STATUS, L"bind Error!");
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        } else if (SOCKET_ERROR == listen(listenSocket, SOMAXCONN)) {
            gui.setText(IDC_TCP_SERVER_STATUS, L"listen Error!", Gui::wsShow);
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        } else
            listenThread = thread(&TCPServer::mainLoop, this);
    }
}
void TCPServer::destroy() {
    mainLoopIsRunning = false;
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    this_thread::sleep_for(100ms);
    if (listenThread.joinable()) listenThread.join();
}

void TCPServer::setServerIPs(uint16_t port) {
    char ipBuf[INET_ADDRSTRLEN];
    hostent *phe = nullptr;

    if (gethostname(ipBuf, sizeof(ipBuf)) == SOCKET_ERROR || (phe = gethostbyname(ipBuf)) == nullptr) {
        gui.setText(IDC_TCP_SERVER_STATUS, L"gethostname Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }
    string server_ip = "";
    in_addr server_addr = {0};
    int i = 0;
    while (phe->h_addr_list[i] != nullptr) {
        memcpy(&server_addr, phe->h_addr_list[i++], sizeof(in_addr));
        if (!server_ip.empty()) server_ip += "\n";
        server_ip += inet_ntop(AF_INET, &server_addr, ipBuf, sizeof(ipBuf)) + (":" + to_string(port));
    }
    gui.setText(IDC_TCP_SERVER_IP, wstring(server_ip.begin(), server_ip.end()));
}
