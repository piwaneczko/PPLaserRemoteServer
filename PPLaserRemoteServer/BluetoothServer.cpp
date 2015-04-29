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
serverSocket(INVALID_SOCKET),
clientSocket(INVALID_SOCKET)
{
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
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
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

    serverSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (serverSocket == INVALID_SOCKET) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"socket Error!", GUI::wsShow);
        listenThread.detach();
        return;
    }

    SOCKADDR_BTH    sockAddrBthLocal    = { 0 };
    int             sockAddrBthLocalLen = sizeof(SOCKADDR_BTH);
    sockAddrBthLocal.addressFamily = AF_BTH;
    sockAddrBthLocal.port = BT_PORT_ANY;

    if (SOCKET_ERROR == ::bind(serverSocket,
        (struct sockaddr *) &sockAddrBthLocal,
        sizeof(SOCKADDR_BTH))) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"bind Error!", GUI::wsShow);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    if (SOCKET_ERROR == getsockname(serverSocket,
        (struct sockaddr *)&sockAddrBthLocal,
        &sockAddrBthLocalLen)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"getsockname Error!", GUI::wsShow);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
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
    wsaQuerySet.lpszComment = L"PowerPoint Laser Remote Server";
    wsaQuerySet.dwNameSpace = NS_BTH;
    wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
    wsaQuerySet.lpcsaBuffer = &clientSocketAddrInfo;  // Req'd.

    if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"WSASetService Error!", GUI::wsShow);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    if (SOCKET_ERROR == listen(serverSocket, DEFAULT_LISTEN_BACKLOG)) {
        gui.SetText(IDC_BTH_SERVER_STATUS, L"listen Error!", GUI::wsShow);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    uint8_t recv_buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    clientSocket = INVALID_SOCKET;
    SOCKADDR_BTH clientAddr = { 0 };
    int clientAddrLen = sizeof(SOCKADDR_BTH);

    listenThreadIsRunning = true;
    gui.SetText(IDC_BTH_CLIENT_NAME, L"None", GUI::wsTimedHide);
    while (listenThreadIsRunning) {

        gui.SetText(IDC_BTH_CLIENT_NAME, L"None");
        gui.SetText(IDC_BTH_SERVER_STATUS, L"Listening...");

        clientSocket = accept(serverSocket,
            (struct sockaddr *)&clientAddr,
            &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (listenThreadIsRunning) {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"accept Error!", GUI::wsShow);
                closesocket(serverSocket);
                serverSocket = INVALID_SOCKET;
            }
            else {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"Ending listening");
            }
            break; // Break out of the for loop
        }

        gui.SetText(IDC_BTH_SERVER_STATUS, L"Connected");
        gui.SetText(IDC_BTH_CLIENT_NAME, BthAddrToName(clientAddr));
        gui.Connected(this);

        while (listenThreadIsRunning) {
            recvResult = recv(clientSocket, (char *)recv_buff, RECV_BUFF_MAX_LEN, 0);

            if (recvResult == 0) {  // socket connection has been closed gracefully
                gui.SetText(IDC_BTH_SERVER_STATUS, L"Device connection was lost!");
                break; // Break out of the for loop
            }
            else if (recvResult == SOCKET_ERROR) {
                gui.SetText(IDC_BTH_SERVER_STATUS, L"recv Error!", GUI::wsShow);
                break; // Break out of the for loop
            }
            else {
                //odebrano dane
                gui.ProcRecvData(this, recv_buff, (uint16_t)recvResult);
            }
        }
        gui.Disconnected(this);
        Sleep(1000);
    }
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    listenThreadIsRunning = false;
}