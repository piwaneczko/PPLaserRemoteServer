/**
* \brief èrÛd≥o klasy serwera Bluetooth
* \file TCPServer.cpp
* \author Pawe≥ Iwaneczko
*/
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "TCPServer.h"

using namespace std;

bool CheckPortTCP(uint16_t dwPort)
{
    sockaddr_in client;
    SOCKET sock = INVALID_SOCKET;
    bool result = false;

    memset((void*)&client, 0, sizeof(client)); // czyszczenie pamiÍci
    client.sin_family = AF_INET;
    client.sin_port = htons(dwPort);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = (int)socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) { // nie udala sie inicjalizacja socketa
        printf("socket failed with error: %ld\n", WSAGetLastError());
    }

    if (connect(sock, (struct sockaddr *) &client, sizeof(client)) == SOCKET_ERROR)
    {
        int iResult = WSAGetLastError();
        if (iResult == WSAECONNREFUSED) {
            result = true;
        }
        else
            printf("connect to port %d failed with error: %d\n", dwPort, WSAGetLastError());
    }

    closesocket(sock);

    return result;
}

TCPServer::TCPServer(GUI &gui) :
gui(gui),
listenThreadIsRunning()
{
    gui.SetText(IDC_TCP_SERVER_STATUS, L"Initialization...");
    listenThread = thread(&TCPServer::ListenThread, this);
}
TCPServer::~TCPServer() {
    if (listenThreadIsRunning) {
        listenThreadIsRunning = false;
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        listenThread.join();
    }
}
void TCPServer::ListenThread()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"WSAStartup error!");
        return;
    }

    uint16_t port = 5555;
    while (!CheckPortTCP(port)) port++;

    wchar_t         szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerName(szThisComputerName, &dwLenComputerName)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"GetComputerName Error!");
        listenThread.detach();
        return;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"socket Error!");
        listenThread.detach();
        return;
    }

    sockaddr_in     sockAddrTcpLocal    = { 0 };
    int             sockAddrTcpLocalLen = sizeof(sockaddr_in);
    sockAddrTcpLocal.sin_family = AF_INET;
    sockAddrTcpLocal.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddrTcpLocal.sin_port = htons(port);

    if (SOCKET_ERROR == ::bind(listenSocket,
        (struct sockaddr *) &sockAddrTcpLocal,
        sockAddrTcpLocalLen)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"bind Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    if (SOCKET_ERROR == getsockname(listenSocket,
        (struct sockaddr *)&sockAddrTcpLocal,
        &sockAddrTcpLocalLen)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"getsockname Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    WSAQUERYSET     wsaQuerySet = { 0 };
    CSADDR_INFO     clientSocketAddrInfo = { 0 };
    clientSocketAddrInfo.LocalAddr.iSockaddrLength = sizeof(sockaddr_in);
    clientSocketAddrInfo.LocalAddr.lpSockaddr = (LPSOCKADDR)&sockAddrTcpLocal;
    clientSocketAddrInfo.RemoteAddr.iSockaddrLength = sizeof(sockaddr_in);
    clientSocketAddrInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR)&sockAddrTcpLocal;
    clientSocketAddrInfo.iSocketType = SOCK_STREAM;
    clientSocketAddrInfo.iProtocol = IPPROTO_TCP;

    ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
    wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
    wsaQuerySet.lpServiceClassId = (LPGUID)&guidServiceClass;

    wsaQuerySet.lpszServiceInstanceName = (LPWSTR)(wstring(szThisComputerName) + L" " + REMOTE_SERVER_INSTANCE_STRING).c_str();
    wsaQuerySet.lpszComment = L"PowerPoint Laser Remote Server";
    wsaQuerySet.dwNameSpace = NS_ALL;
    wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
    wsaQuerySet.lpcsaBuffer = &clientSocketAddrInfo;  // Req'd.

    if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"WSASetService Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    char ac[80];
    hostent *phe = NULL;
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR || 
        (phe = gethostbyname(ac)) == NULL){
        gui.SetText(IDC_TCP_SERVER_STATUS, L"gethostname Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }
    string server_ip = "";
    in_addr server_addr = { 0 };
    int i = 0;
    while (phe->h_addr_list[i] != NULL) {
        memcpy(&server_addr, phe->h_addr_list[i++], sizeof(in_addr));
        if (!server_ip.empty()) server_ip += "\n";
        server_ip += inet_ntoa(server_addr) + string(port != 5555 ? ":" + to_string(port) : "");
    }
    gui.SetText(IDC_TCP_SERVER_IP, wstring(server_ip.begin(), server_ip.end()));

    if (SOCKET_ERROR == listen(listenSocket, DEFAULT_LISTEN_BACKLOG)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"listen Error!");
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    uint8_t recv_buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    SOCKET clientSocket = INVALID_SOCKET;
    sockaddr_in clientAddr = { 0 };
    int clientAddrLen = sizeof(sockaddr_in);

    Sleep(1000);
    listenThreadIsRunning = true;
    while (listenThreadIsRunning) {

        gui.SetText(IDC_TCP_CLIENT_IP, L"None");
        gui.SetText(IDC_TCP_SERVER_STATUS, L"Listening...");

        clientSocket = accept(listenSocket,
            (struct sockaddr *)&clientAddr,
            &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (listenThreadIsRunning) {
                gui.SetText(IDC_TCP_SERVER_STATUS, L"accept Error!");
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
            }
            else {
                gui.SetText(IDC_TCP_SERVER_STATUS, L"Ending listening");
            }
            break; // Break out of the for loop
        }

        gui.SetText(IDC_TCP_SERVER_STATUS, L"Connected");
        string clientIP = inet_ntoa(clientAddr.sin_addr);
        gui.SetText(IDC_TCP_CLIENT_IP, wstring(clientIP.begin(), clientIP.end()));

        while (listenThreadIsRunning) {
            recvResult = recv(clientSocket, (char *)recv_buff, RECV_BUFF_MAX_LEN, 0);

            if (recvResult == 0) {  // socket connection has been closed gracefully

                gui.SetText(IDC_TCP_SERVER_STATUS, L"Device connection was lost!");
                Sleep(1000);
                break; // Break out of the for loop
            }
            else if (recvResult == SOCKET_ERROR) {

                gui.SetText(IDC_TCP_SERVER_STATUS, L"recv Error!");
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
                break; // Break out of the for loop
            }
            else {
                //odebrano dane
                gui.ProcRecvData(recv_buff, (uint16_t)recvResult);
            }
        }
    }
    if (INVALID_SOCKET != clientSocket) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }

    if (INVALID_SOCKET != listenSocket) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    listenThreadIsRunning = false;
}