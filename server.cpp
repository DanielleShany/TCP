#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <Ws2tcpip.h> // Include for InetNtop
#include "http_utils.h"

#pragma comment(lib, "Ws2_32.lib")


bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
void checkTimeouts();

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

void main()
{
    // Initialize Winsock (Windows Sockets).
    WSAData wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        std::cout << "HTTP Server: Error at WSAStartup()\n";
        return;
    }

    // Create a SOCKET object called listenSocket.
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listenSocket)
    {
        std::cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Create a sockaddr_in object called serverService.
    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(HTTP_PORT);

    // Bind the socket for client's requests.
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
    {
        std::cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Listen on the Socket for incoming connections.
    if (SOCKET_ERROR == listen(listenSocket, 5))
    {
        std::cout << "HTTP Server: Error at listen(): " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return;
    }
    addSocket(listenSocket, LISTEN);

    // Accept connections and handle them one by one.
    while (true)
    {
        fd_set waitRecv;
        FD_ZERO(&waitRecv);
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
                FD_SET(sockets[i].id, &waitRecv);
        }

        fd_set waitSend;
        FD_ZERO(&waitSend);
        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i].send == SEND)
                FD_SET(sockets[i].id, &waitSend);
        }

        timeval timeout;
        timeout.tv_sec = 2 * 60; // 2 minutes
        timeout.tv_usec = 0;

        int nfd = select(0, &waitRecv, &waitSend, NULL, &timeout);
        if (nfd == SOCKET_ERROR)
        {
            std::cout << "HTTP Server: Error at select(): " << WSAGetLastError() << std::endl;
            WSACleanup();
            return;
        }

        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
        {
            if (FD_ISSET(sockets[i].id, &waitRecv))
            {
                nfd--;
                switch (sockets[i].recv)
                {
                case LISTEN:
                    acceptConnection(i);
                    break;

                case RECEIVE:
                    receiveMessage(i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
        {
            if (FD_ISSET(sockets[i].id, &waitSend))
            {
                nfd--;
                switch (sockets[i].send)
                {
                case SEND:
                    sendMessage(i);
                    break;
                }
            }
        }

        // Check for timeouts
        checkTimeouts();
    }

    // Closing connections and Winsock.
    std::cout << "HTTP Server: Closing Connection.\n";
    closesocket(listenSocket);
    WSACleanup();
}

bool addSocket(SOCKET id, int what)
{
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (sockets[i].recv == EMPTY)
        {
            sockets[i].id = id;
            sockets[i].recv = what;
            sockets[i].send = IDLE;
            sockets[i].len = 0;
            sockets[i].lastActivity = time(NULL); // Initialize last activity time
            socketsCount++;
            return true;
        }
    }
    return false;
}

void removeSocket(int index)
{
    sockets[index].recv = EMPTY;
    sockets[index].send = EMPTY;
    socketsCount--;
}

void acceptConnection(int index)
{
    SOCKET id = sockets[index].id;
    struct sockaddr_in from;        // Address of sending partner
    int fromLen = sizeof(from);

    SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
    if (INVALID_SOCKET == msgSocket)
    {
        std::cout << "HTTP Server: Error at accept(): " << WSAGetLastError() << std::endl;
        return;
    }

    wchar_t ipStr[INET_ADDRSTRLEN];
    InetNtop(AF_INET, &from.sin_addr, ipStr, INET_ADDRSTRLEN);
    std::cout << "HTTP Server: Client " << ipStr << ":" << ntohs(from.sin_port) << " is connected." << std::endl;

    // Set the socket to be in non-blocking mode.
    unsigned long flag = 1;
    if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
    {
        std::cout << "HTTP Server: Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
    }

    if (addSocket(msgSocket, RECEIVE) == false)
    {
        std::cout << "\t\tToo many connections, dropped!\n";
        closesocket(id);
    }
    return;
}

void receiveMessage(int index)
{
    SOCKET msgSocket = sockets[index].id;

    int len = sockets[index].len;
    int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

    if (SOCKET_ERROR == bytesRecv)
    {
        std::cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << std::endl;
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    if (bytesRecv == 0)
    {
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    else
    {
        sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
        std::cout << "HTTP Server: Received: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

        sockets[index].len += bytesRecv;
        sockets[index].lastActivity = time(NULL); // Update last activity time

        if (sockets[index].len > 0)
        {
            handleHttpRequest(sockets[index]);
        }
    }
}

void sendMessage(int index)
{
    int bytesSent = 0;
    SOCKET msgSocket = sockets[index].id;

    bytesSent = send(msgSocket, sockets[index].buffer, sockets[index].len, 0);
    if (SOCKET_ERROR == bytesSent)
    {
        std::cout << "HTTP Server: Error at send(): " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "HTTP Server: Sent: " << bytesSent << "\\" << sockets[index].len << " bytes of \"" << sockets[index].buffer << "\" message.\n";

    sockets[index].len -= bytesSent;
    if (sockets[index].len > 0)
    {
        memmove(sockets[index].buffer, sockets[index].buffer + bytesSent, sockets[index].len);
    }
    else
    {
        sockets[index].send = IDLE;
        sockets[index].recv = RECEIVE;
    }
}

void checkTimeouts()
{
    time_t currentTime = time(NULL);
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (sockets[i].recv != EMPTY && sockets[i].recv != LISTEN)
        {
            if (difftime(currentTime, sockets[i].lastActivity) > 2 * 60) // 2 minutes
            {
                std::cout << "HTTP Server: Closing connection due to inactivity.\n";
                closesocket(sockets[i].id);
                removeSocket(i);
            }
        }
    }
}
