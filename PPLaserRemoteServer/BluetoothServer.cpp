/**
 * \brief èrÛd≥o klasy serwera Bluetooth
 * \file BluetoothServer.cpp
 * \author Pawe≥ Iwaneczko
 */
#include <stdio.h>
#include <WinSock2.h>
#include <ws2bth.h>
#include "BluetoothServer.h"

using namespace std;

#define RECV_BUFF_MAX_LEN                 19      /**< Maksymalny rozmiar bufora odebranych danych */

BluetoothServer::BluetoothServer(GUI &gui) :
gui(gui),
listenThreadIsRunning(),
listenSocket(INVALID_SOCKET),
clientSocket(INVALID_SOCKET)
{
    serverType = stBluetooth;
    gui.SetText(IDC_BTH_SERVER_STATUS, L"Initialization...");
    listenThread = thread(&BluetoothServer::ListenThread, this);
}
BluetoothServer::~BluetoothServer() {
    if (listenThreadIsRunning) {
        listenThreadIsRunning = false;
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
wstring BthAddrToName(const SOCKADDR_BTH &remoteBthAddr) {
    int             iResult = 0;
    uint32_t        flags = LUP_CONTAINERS | LUP_RETURN_NAME | LUP_RETURN_ADDR;
    DWORD           qsSize = sizeof(WSAQUERYSET);
    PWSAQUERYSET    pWSAQuerySet = NULL;
    HANDLE          lookup = NULL;
    wstring         result;

    pWSAQuerySet = (PWSAQUERYSET) HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            qsSize);
    ZeroMemory(pWSAQuerySet, qsSize);
    pWSAQuerySet->dwNameSpace = NS_BTH;
    pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);

    iResult = WSALookupServiceBegin(pWSAQuerySet, flags, &lookup);
    if ((NO_ERROR == iResult) && (NULL != lookup)) {
        while (true) {
            if (NO_ERROR == WSALookupServiceNext(lookup,
                flags,
                &qsSize,
                pWSAQuerySet)) 
            {
                if ((pWSAQuerySet->lpszServiceInstanceName != NULL) && 
                    (((PSOCKADDR_BTH)pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr == remoteBthAddr.btAddr)) 
                {
                    result = pWSAQuerySet->lpszServiceInstanceName;
                    break;
                }
            }
            else {
                iResult = WSAGetLastError();
                if (WSA_E_NO_MORE == iResult) { //No more data
                    break;
                }
                else if (WSAEFAULT == iResult) {
                    //Potrzebny wiÍkszy rozmiar
                    HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
                    pWSAQuerySet = (PWSAQUERYSET)HeapAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY,
                        qsSize);
                    if (NULL == pWSAQuerySet) {
                        //wprintf(L"!ERROR! | Unable to allocate memory for WSAQERYSET\n");
                        iResult = STATUS_NO_MEMORY;
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
        WSALookupServiceEnd(lookup);
    }

    if (NULL != pWSAQuerySet) {
        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
        pWSAQuerySet = NULL;
    }

    return result;
}

void BluetoothServer::ListenThread()
{
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"WSAStartup error!", GUI::wsShow);
        return;
    }

    wchar_t         szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerName(szThisComputerName, &dwLenComputerName)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"GetComputerName Error!", GUI::wsShow);
        listenThread.detach();
        return;
    }
    else {
        gui.SetText(IDC_BTH_SERVER_NAME, szThisComputerName);
    }

    listenSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (listenSocket == INVALID_SOCKET) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"socket Error!", GUI::wsShow);
        listenThread.detach();
        return;
    }

    SOCKADDR_BTH    sockAddrBthLocal    = { 0 };
    int             sockAddrBthLocalLen = sizeof(SOCKADDR_BTH);
    sockAddrBthLocal.addressFamily = AF_BTH;
    sockAddrBthLocal.port = BT_PORT_ANY;

    if (SOCKET_ERROR == ::bind(listenSocket,
        (struct sockaddr *) &sockAddrBthLocal,
        sockAddrBthLocalLen)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"No Bluetooth Device!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    if (SOCKET_ERROR == getsockname(listenSocket,
        (struct sockaddr *)&sockAddrBthLocal,
        &sockAddrBthLocalLen)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"getsockname Error!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    WSAQUERYSET     wsaQuerySet = { 0 };
    CSADDR_INFO     clientSocketAddrInfo = { 0 };
    clientSocketAddrInfo.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    clientSocketAddrInfo.LocalAddr.lpSockaddr = (LPSOCKADDR)&sockAddrBthLocal;
    clientSocketAddrInfo.RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    clientSocketAddrInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR)&sockAddrBthLocal;
    clientSocketAddrInfo.iSocketType = SOCK_STREAM;
    clientSocketAddrInfo.iProtocol = BTHPROTO_RFCOMM;

    ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
    wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
    wsaQuerySet.lpServiceClassId = (LPGUID)&guidServiceClass;

    wsaQuerySet.lpszServiceInstanceName = (LPWSTR)(wstring(szThisComputerName) + L" " + REMOTE_SERVER_INSTANCE_STRING).c_str();
    wsaQuerySet.lpszComment = L"PP Laser Remote Server";
    wsaQuerySet.dwNameSpace = NS_BTH;
    wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
    wsaQuerySet.lpcsaBuffer = &clientSocketAddrInfo;  // Req'd.

    if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"WSASetService Error!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    if (SOCKET_ERROR == listen(listenSocket, SOMAXCONN)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"listen Error!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    uint16_t offs = 0, len = 0, dlen = 0;
    uint8_t buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    clientSocket = INVALID_SOCKET;
    SOCKADDR_BTH clientAddr = { 0 };
    int clientAddrLen = sizeof(SOCKADDR_BTH);

    int numReady;
    FD_SET fdrecv;
    timeval timeout =       { 0,  10000 }; // 10ms

    listenThreadIsRunning = true;
    gui.SetText(IDC_BTH_CLIENT_NAME, L"None", GUI::wsTimedHide);
    while (listenThreadIsRunning) {

        gui.SetText(IDC_BTH_CLIENT_NAME, L"None");
        gui.SetText(IDC_BTH_SERVER_STATUS, L"Listening...");

        clientAddr = { 0 };
        clientAddrLen = sizeof(SOCKADDR_BTH);
        clientSocket = accept(listenSocket,
            (struct sockaddr *)&clientAddr,
            &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (listenThreadIsRunning) {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"accept Error!", GUI::wsShow);
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
            }
            else {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"Ending listening");
            }
            break; // Break out of the for loop
        }

        gui.SetText(IDC_BTH_SERVER_STATUS, L"Connected");
        wstring clientName = BthAddrToName(clientAddr);
        gui.SetText(IDC_BTH_CLIENT_NAME, clientName);
        gui.Connected(this);

        //wysy≥πnie wersji softu
        file_version_t ver;
        if (UpdateDownloader::GetFileVersion(ver)) {
            buff[0] = msg_type_version;
            buff[1] = (uint8_t)((ver.major & 0xFF00) >> 8);
            buff[2] = (uint8_t)((ver.major & 0x00FF)     );
            buff[3] = (uint8_t)((ver.minor & 0xFF00) >> 8);
            buff[4] = (uint8_t)((ver.minor & 0x00FF)     );
            send(clientSocket, (const char *)buff, 5, 0);
        }
        while (listenThreadIsRunning) {
            FD_ZERO(&fdrecv);
            FD_SET(clientSocket, &fdrecv);
            numReady = select(0, &fdrecv, nullptr, nullptr, &timeout);
            if (numReady < 0) {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"recv Error!", GUI::wsShow);
                break;
            }
            else if (numReady > 0 && FD_ISSET(clientSocket, &fdrecv) != 0) // jest cos do odczytania
            {
                offs = 0;
                if (len >= RECV_BUFF_MAX_LEN)
                    len = 0;
                recvResult = recv(clientSocket, (char *)buff + len, RECV_BUFF_MAX_LEN - len, 0);

                if (recvResult == 0) {  // socket connection has been closed gracefully
                    gui.SetText(IDC_BTH_SERVER_STATUS, L"Device connection was lost!");
                    break; // Break out of the for loop
                }
                else if (recvResult == SOCKET_ERROR) {
                    gui.SetText(IDC_BTH_SERVER_STATUS, L"recv Error!", GUI::wsShow);
                    break; // Break out of the for loop
                }
                else {
                    len += recvResult;
                    //odebrano dane - dzielenie przetwarzanie
                    while (len >= 4) {
                        dlen = GetDataLen((msg_type_t)buff[offs]);

                        if (dlen > len)
                            break;

                        gui.ProcRecvData(this, &buff[offs], dlen);
                        offs += dlen;
                        len -= dlen;
                    }
                    if (len > 0) {
                        memcpy(&buff[0], &buff[offs], len);
                    }
                }
            }
        }
        gui.Disconnected(this);
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
    }
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    listenThreadIsRunning = false;
}