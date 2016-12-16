/**
* \brief èrÛd≥o klasy serwera Bluetooth
* \file TCPServer.cpp
* \author Pawe≥ Iwaneczko
*/
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "TCPServer.h"
#include "XmlConfig.hpp"

using namespace std;

#define RECV_BUFF_MAX_LEN                 22      /**< Maksymalny rozmiar bufora odebranych danych */

XmlConfigValue<uint16_t> DefaultPort("TcpDefaultPort", 3389);

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
    serverType = stTCP;
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

    uint16_t port = DefaultPort;
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

    if (SOCKET_ERROR == listen(listenSocket, SOMAXCONN)) {
        gui.SetText(IDC_TCP_SERVER_STATUS, L"listen Error!", GUI::wsShow);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        listenThread.detach();
        return;
    }

    uint16_t offs = 0, len = 0, dlen = 0;
    uint8_t buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    clientSocket = INVALID_SOCKET;
    sockaddr_in clientAddr = { 0 };
    int clientAddrLen = sizeof(sockaddr_in);

    int numReady;
    FD_SET fdrecv, listenfd;
    timeval timeout =       { 0,  10000 }; // 10ms
    timeval listenTimeout = { 0, 200000 }; //200Ms

    listenThreadIsRunning = true;
    gui.SetText(IDC_TCP_CLIENT_IP, L"None", GUI::wsTimedHide);
    while (listenThreadIsRunning) {
        SetServerIPs(port);
        gui.SetText(IDC_TCP_CLIENT_IP, L"None");
        gui.SetText(IDC_TCP_SERVER_STATUS, L"Listening...");

        FD_ZERO(&listenfd);
        FD_SET(listenSocket, &listenfd);
        numReady = select(0, &listenfd, NULL, NULL, &listenTimeout);
        if (FD_ISSET(listenSocket, &listenfd) && numReady) {
            clientAddr = { 0 };
            clientAddrLen = sizeof(sockaddr_in);
            ZeroMemory(&clientAddr, sizeof(clientAddr));
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

            //wysy≥πnie wersji softu
            file_version_t ver;
            if (UpdateDownloader::GetFileVersion(ver)) {
                buff[0] = msg_type_version;
                buff[1] = (uint8_t)((ver.major & 0xFF00) >> 8);
                buff[2] = (uint8_t)((ver.major & 0x00FF));
                buff[3] = (uint8_t)((ver.minor & 0xFF00) >> 8);
                buff[4] = (uint8_t)((ver.minor & 0x00FF));
                send(clientSocket, (const char *)buff, 5, 0);
            }
            while (listenThreadIsRunning) {
                FD_ZERO(&fdrecv);
                FD_SET(clientSocket, &fdrecv);
                numReady = select(0, &fdrecv, nullptr, nullptr, &timeout);
                if (numReady < 0) {
                    gui.SetText(IDC_TCP_SERVER_STATUS, L"recv Error!", GUI::wsShow);
                    break;
                }
                else if (numReady > 0 && FD_ISSET(clientSocket, &fdrecv) != 0) // jest cos do odczytania
                {
                    offs = 0;
                    if (len >= RECV_BUFF_MAX_LEN)
                        len = 0;
                    recvResult = recv(clientSocket, (char *)buff + len, RECV_BUFF_MAX_LEN - len, 0);
                    if (recvResult == 0) {  // socket connection has been closed gracefully
                        gui.SetText(IDC_TCP_SERVER_STATUS, L"Device connection was lost!");
                        break; // Break out of the for loop
                    }
                    else if (recvResult == SOCKET_ERROR) {
                        gui.SetText(IDC_TCP_SERVER_STATUS, L"recv Error!", GUI::wsShow);
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
            if (INVALID_SOCKET != clientSocket) {
                closesocket(clientSocket);
                clientSocket = INVALID_SOCKET;
            }
        }
    }

    if (INVALID_SOCKET != listenSocket) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    listenThreadIsRunning = false;
}

void TCPServer::SetServerIPs(uint16_t port)
{
    char ac[80];
    hostent *phe = NULL;
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR ||
        (phe = gethostbyname(ac)) == NULL) {
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
        server_ip += inet_ntoa(server_addr) + (":" + to_string(port));
    }
    gui.SetText(IDC_TCP_SERVER_IP, wstring(server_ip.begin(), server_ip.end()));
}
