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

#define RECV_BUFF_MAX_LEN                 22      /**< Maksymalny rozmiar bufora odebranych danych */

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
listenThreadIsRunning(),
listenSocket(INVALID_SOCKET),
clientSocket(INVALID_SOCKET)
{
    gui.SetText(IDC_TCP_SERVER_STATUS, L"Initialization...");
    listenThread = thread(&TCPServer::ListenThread, this);
}
TCPServer::~TCPServer() {
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
void TCPServer::ListenThread()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"WSAStartup error!", GUI::wsShow);
        return;
    }

    uint16_t port = 5555;
    while (!CheckPortTCP(port)) port++;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"socket Error!", GUI::wsShow);
        listenThread.detach();
        return;
    }

    sockaddr_in     sockAddrTcpLocal    = { 0 };
    int             sockAddrTcpLocalLen = sizeof(sockaddr_in);
    sockAddrTcpLocal.sin_family = AF_INET;
    sockAddrTcpLocal.sin_addr.s_addr = inet_addr("0.0.0.0");
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
        gui.SetText(IDC_TCP_SERVER_STATUS, L"listen Error!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    uint8_t procData[RECV_BUFF_MAX_LEN] = { 0 };
    uint16_t clen = 0, len = 0;
    clock_t timeCounter = 0;
    uint8_t recv_buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    clientSocket = INVALID_SOCKET;
    sockaddr_in clientAddr = { 0 };
    int clientAddrLen = sizeof(sockaddr_in);

    listenThreadIsRunning = true;
    gui.SetText(IDC_TCP_CLIENT_IP, L"None", GUI::wsTimedHide);
    while (listenThreadIsRunning) {

        gui.SetText(IDC_TCP_CLIENT_IP, L"None");
        gui.SetText(IDC_TCP_SERVER_STATUS, L"Listening...");

        clientSocket = accept(listenSocket,
            (struct sockaddr *)&clientAddr,
            &clientAddrLen);
        if (INVALID_SOCKET == clientSocket) {
            if (listenThreadIsRunning) {
                gui.SetText(IDC_TCP_SERVER_STATUS, L"accept Error!", GUI::wsShow);
                closesocket(listenSocket);
                listenSocket = INVALID_SOCKET;
            }
            else {
                gui.SetText(IDC_TCP_SERVER_STATUS, L"Ending listening", GUI::wsShow);
            }
            break; // Break out of the for loop
        }

        gui.SetText(IDC_TCP_SERVER_STATUS, L"Connected");
        string clientIP = inet_ntoa(clientAddr.sin_addr);
        gui.SetText(IDC_TCP_CLIENT_IP, wstring(clientIP.begin(), clientIP.end()));
        gui.Connected(this);
        while (listenThreadIsRunning) {
            recvResult = recv(clientSocket, (char *)recv_buff, RECV_BUFF_MAX_LEN, 0);
            if (recvResult == 0) {  // socket connection has been closed gracefully
                gui.SetText(IDC_TCP_SERVER_STATUS, L"Device connection was lost!");
                break; // Break out of the for loop
            }
            else if (recvResult == SOCKET_ERROR) {
                gui.SetText(IDC_TCP_SERVER_STATUS, L"recv Error!", GUI::wsShow);
                break; // Break out of the for loop
            }
            else {
                //odebrano dane - dzielenie przetwarzanie
                for (i = 0; i < recvResult; i++)
                {
                    procData[clen++] = recv_buff[i];
                    if (clen == 1) {
                        switch (procData[0])
                        {
                        case msg_type_gesture:
                        case msg_type_gyro:
                            len = 11;
                            break;
                        case msg_type_key:
                        case msg_type_laser:
                        default:
                            len = 4;
                            break;
                        }
                    }
                    if (clen >= len) {
                        gui.ProcRecvData(this, procData, len);
                        clen = 0;
                    }
                    if (clen >= RECV_BUFF_MAX_LEN) //b≥πd
                        clen = 0;
                }
            }
        }

        gui.Disconnected(this);
        Sleep(1000);
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