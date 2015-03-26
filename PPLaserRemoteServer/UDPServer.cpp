/**
* \brief èrÛd≥o klasy serwera Bluetooth
* \file UDPServer.cpp
* \author Pawe≥ Iwaneczko
*/
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "UDPServer.h"

using namespace std;

uint8_t hellomsg[6] = { 0xFF, 0xEE, 0x77, 0x88, 0x3A, 0x95 };
uint8_t byemsg[6] = { 0xBB, 0xAA, 0xEE, 0x00, 0x73, 0x52 };

UDPServer::UDPServer(GUI &gui) :
gui(gui),
listenThreadIsRunning(),
serverSocket(INVALID_SOCKET),
clientIP("")
{
    gui.SetText(IDC_UDP_SERVER_STATUS, L"Initialization...");
    listenThread = thread(&UDPServer::ListenThread, this);
}
UDPServer::~UDPServer() {
    if (listenThreadIsRunning) {
        listenThreadIsRunning = false;
        if (serverSocket != INVALID_SOCKET) {
            if (!clientIP.empty())
                sendto(serverSocket, (char *)byemsg, 6, 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
            WSACleanup();
        }
        listenThread.join();
    }
}

void UDPServer::ListenThread()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        gui.SetText(IDC_UDP_SERVER_STATUS, L"WSAStartup error!");
        return;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        gui.SetText(IDC_UDP_SERVER_STATUS, L"Network not available!");
        listenThread.detach();
        return;
    }

	uint16_t port = 5555;
    sockaddr_in     sockAddrUDPLocal    = { 0 };
    int             sockAddrUDPLocalLen = sizeof(sockaddr_in);
    sockAddrUDPLocal.sin_family = AF_INET;
    sockAddrUDPLocal.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockAddrUDPLocal.sin_port = htons(port);

    while (SOCKET_ERROR == ::bind(serverSocket,
        (struct sockaddr *) &sockAddrUDPLocal,
		sockAddrUDPLocalLen)) {
		sockAddrUDPLocal.sin_port = htons(++port);

		if (port > 5559) {
			gui.SetText(IDC_UDP_SERVER_STATUS, L"bind Error!");
			closesocket(serverSocket);
			serverSocket = INVALID_SOCKET;
			listenThread.detach();
			return;
		}
    }

    char ac[80];
    hostent *phe = NULL;
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR || 
        (phe = gethostbyname(ac)) == NULL){
        gui.SetText(IDC_UDP_SERVER_STATUS, L"gethostname Error!");
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
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
    gui.SetText(IDC_UDP_SERVER_IP, wstring(server_ip.begin(), server_ip.end()));

    uint8_t recv_buff[RECV_BUFF_MAX_LEN] = { 0 };
    int recvResult = 0;
    sockaddr_in clientAddr = { 0 };
    int clientAddrLen = sizeof(sockaddr_in);

    listenThreadIsRunning = true;
    while (listenThreadIsRunning) {

        gui.SetText(IDC_UDP_CLIENT_IP, L"None");
        gui.SetText(IDC_UDP_SERVER_STATUS, L"Listening...");

        this->clientIP = "";
        this->clientAddr = { 0 };
        this->clientAddrLen = sizeof(sockaddr_in);

		recvResult = recvfrom(serverSocket, (char *)recv_buff, RECV_BUFF_MAX_LEN, 0,
			(struct sockaddr *)&this->clientAddr,
            &this->clientAddrLen);

        if (recvResult == 6 && !memcmp(recv_buff, hellomsg, 6)) {
            //wys≥anie potwierdzenia po≥πczenia
            if(sendto(serverSocket, (char *)hellomsg, 6, 0, (struct sockaddr *)&this->clientAddr, this->clientAddrLen) != 6)
                continue;

			//po≥πczono z klientem, zapisanie ip (tylko jeden user)
			gui.SetText(IDC_UDP_SERVER_STATUS, L"Connected");
            clientIP = inet_ntoa(this->clientAddr.sin_addr);
			gui.SetText(IDC_UDP_CLIENT_IP, wstring(clientIP.begin(), clientIP.end()));
			gui.Connected(this);

			while (listenThreadIsRunning) {
				recvResult = recvfrom(serverSocket, (char *)recv_buff, RECV_BUFF_MAX_LEN, 0,
					(struct sockaddr *)&clientAddr,
					&clientAddrLen);

                if (string(inet_ntoa(clientAddr.sin_addr)) != clientIP)
                    continue;

                if (recvResult == 0 || (recvResult == 6 && !memcmp(recv_buff, byemsg, 6))) {  
                    // socket connection has been closed gracefully
					gui.SetText(IDC_UDP_SERVER_STATUS, L"Device connection was lost!");
					break; // Break out of the for loop
				}
				else if (recvResult == SOCKET_ERROR) {
					gui.SetText(IDC_UDP_SERVER_STATUS, L"recv Error!");
					break; // Break out of the for loop
				}
				else {
					//odebrano dane
					gui.ProcRecvData(this, recv_buff, (uint16_t)recvResult);
				}
			}

			gui.Disconnected(this);
		}

        Sleep(1000);
    }

    if (INVALID_SOCKET != serverSocket) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    listenThreadIsRunning = false;
}