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

const int HTTP_PORT = 80;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

void handleHttpRequest(SocketState& socket);

#endif // HTTP_UTILS_H

