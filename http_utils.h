#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <string>

struct SocketState
{
    SOCKET id;         // Socket handle
    int recv;          // Receiving?
    int send;          // Sending?
    int sendSubType;   // Sending sub-type
    char buffer[128];
    int len;
    time_t lastActivity; // Time of the last activity
};

constexpr int HTTP_PORT = 80;
constexpr int MAX_SOCKETS = 60;
constexpr int EMPTY = 0;
constexpr int LISTEN = 1;
constexpr int RECEIVE = 2;
constexpr int IDLE = 3;
constexpr int SEND = 4;

void handleHttpRequest(SocketState& socket);

#endif // HTTP_UTILS_H

