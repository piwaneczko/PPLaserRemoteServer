/**
 * \brief èrÛd≥o klasy serwera Bluetooth
 * \file BluetoothServer.cpp
 * \author Pawe≥ Iwaneczko
 */
#include <WinSock2.h>
#include <stdio.h>
#include <ws2bth.h>
// BluetoothServer.hpp include need to be agter ws2tcpip
#include "BluetoothServer.hpp"
#include "resource.h"

using namespace std;

#define RECV_BUFF_MAX_LEN 19 /**< Maksymalny rozmiar bufora odebranych danych */

BluetoothServer::BluetoothServer(Gui &gui) : Server(gui) {
    serverType = stBluetooth;
    gui.setText(IDC_BTH_SERVER_STATUS, L"Initialization...");

    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData)) {
        gui.setText(IDC_BTH_SERVER_STATUS, L"WSAStartup error!", Gui::wsShow);
    }

    wchar_t szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerName(szThisComputerName, &dwLenComputerName)) {
        gui.setText(IDC_BTH_SERVER_STATUS, L"GetComputerName Error!", Gui::wsShow);
    } else {
        gui.setText(IDC_BTH_SERVER_NAME, szThisComputerName);
    }

    listenSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (listenSocket == INVALID_SOCKET) {
        gui.setText(IDC_BTH_SERVER_STATUS, L"socket Error!", Gui::wsShow);
    }

    SOCKADDR_BTH sockAddrBthLocal = {0};
    int sockAddrBthLocalLen = sizeof(SOCKADDR_BTH);
    sockAddrBthLocal.addressFamily = AF_BTH;
    sockAddrBthLocal.port = BT_PORT_ANY;

    if (SOCKET_ERROR == ::bind(listenSocket, (struct sockaddr *)&sockAddrBthLocal, sockAddrBthLocalLen)) {
        gui.setText(IDC_BTH_SERVER_STATUS, L"No Bluetooth Device!", Gui::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    } else if (SOCKET_ERROR == getsockname(listenSocket, reinterpret_cast<sockaddr *>(&sockAddrBthLocal), &sockAddrBthLocalLen)) {
        gui.setText(IDC_BTH_SERVER_STATUS, L"getsockname Error!", Gui::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    } else {
        WSAQUERYSET wsaQuerySet = {0};
        CSADDR_INFO clientSocketAddrInfo = {nullptr};
        clientSocketAddrInfo.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        clientSocketAddrInfo.LocalAddr.lpSockaddr = LPSOCKADDR(&sockAddrBthLocal);
        clientSocketAddrInfo.RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        clientSocketAddrInfo.RemoteAddr.lpSockaddr = LPSOCKADDR(&sockAddrBthLocal);
        clientSocketAddrInfo.iSocketType = SOCK_STREAM;
        clientSocketAddrInfo.iProtocol = BTHPROTO_RFCOMM;

        ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
        wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
        wsaQuerySet.lpServiceClassId = LPGUID(&guidServiceClass);

        wsaQuerySet.lpszServiceInstanceName = (LPWSTR)(wstring(szThisComputerName) + L" " + REMOTE_SERVER_INSTANCE_STRING).c_str();
        wsaQuerySet.lpszComment = L"PP Laser Remote Server";
        wsaQuerySet.dwNameSpace = NS_BTH;
        wsaQuerySet.dwNumberOfCsAddrs = 1;                // Must be 1.
        wsaQuerySet.lpcsaBuffer = &clientSocketAddrInfo;  // Req'd.

        if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
            gui.setText(IDC_BTH_SERVER_STATUS, L"WSASetService Error!", Gui::wsShow);
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        } else if (SOCKET_ERROR == listen(listenSocket, SOMAXCONN)) {
            gui.setText(IDC_BTH_SERVER_STATUS, L"listen Error!", Gui::wsShow);
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        } else
            listenThread = thread(&BluetoothServer::mainLoop, this);
    }
}
BluetoothServer::~BluetoothServer() {
    if (mainLoopIsRunning) {
        mainLoopIsRunning = false;
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        listenThread.join();
    }
}

wstring bthAddrToName(const SOCKADDR_BTH &remoteBthAddr) {
    auto iResult = 0;
    const uint32_t flags = LUP_CONTAINERS | LUP_RETURN_NAME | LUP_RETURN_ADDR;
    DWORD qsSize = sizeof(WSAQUERYSET);
    HANDLE lookup = nullptr;
    wstring result;

    PWSAQUERYSET pWSAQuerySet = PWSAQUERYSET(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, qsSize));
    ZeroMemory(pWSAQuerySet, qsSize);
    pWSAQuerySet->dwNameSpace = NS_BTH;
    pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);

    iResult = WSALookupServiceBegin(pWSAQuerySet, flags, &lookup);
    if ((NO_ERROR == iResult) && (nullptr != lookup)) {
        while (true) {
            if (NO_ERROR == WSALookupServiceNext(lookup, flags, &qsSize, pWSAQuerySet)) {
                if ((pWSAQuerySet->lpszServiceInstanceName != nullptr)
                    && (PSOCKADDR_BTH(pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr == remoteBthAddr.btAddr)) {
                    result = pWSAQuerySet->lpszServiceInstanceName;
                    break;
                }
            } else {
                iResult = WSAGetLastError();
                if (WSA_E_NO_MORE == iResult) {  // No more data
                    break;
                } else if (WSAEFAULT == iResult) {
                    // Potrzebny wiÍkszy rozmiar
                    HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
                    pWSAQuerySet = PWSAQUERYSET(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, qsSize));
                    if (nullptr == pWSAQuerySet) {
                        // wprintf(L"!ERROR! | Unable to allocate memory for WSAQERYSET\n");
                        iResult = STATUS_NO_MEMORY;
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        WSALookupServiceEnd(lookup);
    }

    if (nullptr != pWSAQuerySet) {
        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
        pWSAQuerySet = nullptr;
    }

    return result;
}

void BluetoothServer::mainLoop() {
    uint16_t len = 0;
    uint8_t buff[RECV_BUFF_MAX_LEN] = {0};
    auto recvResult = 0;
    clientSocket = INVALID_SOCKET;
    SOCKADDR_BTH clientAddr = {0};
    int clientAddrLen = sizeof(SOCKADDR_BTH);

    FD_SET fdrecv;
    timeval timeout = {0, 10000};  // 10ms

    gui.setText(IDC_BTH_SERVER_STATUS, L"None", Gui::wsTimedHide);
    mainLoopIsRunning = true;
    while (mainLoopIsRunning) {
        gui.setText(IDC_BTH_CLIENT_NAME, L"None");
        gui.setText(IDC_BTH_SERVER_STATUS, L"Listening...");

        clientAddr = {0};
        clientAddrLen = sizeof(SOCKADDR_BTH);
        clientSocket = accept(listenSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (mainLoopIsRunning) {
                gui.setText(IDC_BTH_SERVER_STATUS, L"accept Error!", Gui::wsShow);
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
            } else {
                gui.setText(IDC_BTH_SERVER_STATUS, L"Ending listening");
            }
            break;  // Break out of the for loop
        }

        gui.setText(IDC_BTH_SERVER_STATUS, L"Connected");
        const auto clientName = bthAddrToName(clientAddr);
        gui.setText(IDC_BTH_CLIENT_NAME, clientName);
        gui.connected(this);

        while (mainLoopIsRunning) {
            FD_ZERO(&fdrecv);
            FD_SET(clientSocket, &fdrecv);
            const auto numReady = select(0, &fdrecv, nullptr, nullptr, &timeout);
            if (numReady < 0) {
                gui.setText(IDC_BTH_SERVER_STATUS, L"recv Error!", Gui::wsShow);
                break;
            } else if (numReady > 0 && FD_ISSET(clientSocket, &fdrecv) != 0)  // jest cos do odczytania
            {
                uint16_t offs = 0;
                if (len >= RECV_BUFF_MAX_LEN) len = 0;
                recvResult = recv(clientSocket, reinterpret_cast<char *>(buff + len), RECV_BUFF_MAX_LEN - len, 0);

                if (recvResult == 0) {  // socket connection has been closed gracefully
                    gui.setText(IDC_BTH_SERVER_STATUS, L"Device connection was lost!");
                    break;  // Break out of the for loop
                } else if (recvResult == SOCKET_ERROR) {
                    gui.setText(IDC_BTH_SERVER_STATUS, L"recv Error!", Gui::wsShow);
                    break;  // Break out of the for loop
                } else {
                    len += recvResult;
                    // odebrano dane - dzielenie przetwarzanie
                    while (len >= 4) {
                        const auto dlen = getDataLen(msg_type_t(buff[offs]));

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
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
    }
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    mainLoopIsRunning = false;
}
