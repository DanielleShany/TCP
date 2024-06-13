
#include "http_utils.h"
#include <iostream>
#include <string.h>

void handleHttpRequest(SocketState& socket)
{
    const char* notSupported = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    const char* okResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

    std::string request(socket.buffer);
    std::string response;

    if (request.find("OPTIONS") == 0)
    {
        response = "HTTP/1.1 200 OK\r\nAllow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE\r\n\r\n";
    }
    else if (request.find("GET") == 0)
    {
        response = okResponse;
        if (request.find("lang=he") != std::string::npos)
        {
            response += "<html><body><h1>מבוא לתקשורת מחשבים - הדף של אדם ודניאל!</h1></body></html>";
        }
        else if (request.find("lang=en") != std::string::npos)
        {
            response += "<html><body><h1>Intro to Computer Communications - It's Danielle and Adam's Page!</h1></body></html>";
        }
        else if (request.find("lang=fr") != std::string::npos)
        {
            response += "<html><body><h1>Introduction aux Communications Informatiques - C'est la page de Danielle et Adam!</h1></body></html>";
        }
        else
        {
            response += "<html><body><h1>Intro to Computer Communications - It's Danielle and Adam's Page!</h1></body></html>";
        }
    }
    else if (request.find("HEAD") == 0)
    {
        response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else if (request.find("POST") == 0)
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nPOST request received";
        std::string::size_type bodyPos = request.find("\r\n\r\n");
        if (bodyPos != std::string::npos)
        {
            std::string body = request.substr(bodyPos + 4);
            std::cout << "POST body: " << body << std::endl;
        }
    }
    else if (request.find("PUT") == 0)
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nPUT request received";
    }
    else if (request.find("DELETE") == 0)
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nDELETE request received";
    }
    else if (request.find("TRACE") == 0)
    {
        response = "HTTP/1.1 200 OK\r\nContent-Type: message/http\r\n\r\n";
        response += request;
    }
    else
    {
        response = notSupported;
    }

    strcpy(socket.buffer, response.c_str());
    socket.len = response.length();
    socket.send = SEND;
    socket.recv = IDLE;
}
