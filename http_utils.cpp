#include "http_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

// Helper function to read file contents into a string
std::string readFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return "<html><body><h1>File not found</h1></body></html>";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper function to create the HTTP response header
void makeHeader(std::string& response, std::string status, std::string contentType)
{
    time_t t;
    time(&t);
    tm* utc_time = gmtime(&t);
    char timeFormatted[80];
    strftime(timeFormatted, sizeof(timeFormatted), "%a, %d %b %Y %H:%M:%S GMT", utc_time);

    std::string lineSuffix("\r\n");
    response = "HTTP/1.1 " + status + lineSuffix;
    response += "Server: HTTP Web Server" + lineSuffix;
    response += "Content-Type: " + contentType + lineSuffix;
    response += "Date: " + std::string(timeFormatted) + lineSuffix;
}

// Helper function to add the HTTP response body
void makeBody(std::string& response, std::string body, bool addSuffix)
{
    std::string suffix("\r\n\r\n");
    response += "Content-Length: " + std::to_string(body.length()) + suffix; // add Content-Length to header.
    // Add Body
    response += body;
    if (addSuffix)
        response += suffix;
}

void handleHttpRequest(SocketState& socket)
{
    const char* notSupported = "405 Method Not Allowed";
    const char* okStatus = "200 OK";
    const char* notFound = "404 Not Found";

    std::string request(socket.buffer);
    std::string response;
    std::string body;
    std::string contentType = "text/html";

    if (request.find("OPTIONS") == 0)
    {
        makeHeader(response, okStatus, "text/plain");
        response += "Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE\r\n\r\n";
    }
    else if (request.find("GET") == 0)
    {
        if (request.find("lang=he") != std::string::npos)
        {
            body = readFile("C:/Users/Dandush/source/repos/TCP/he/text.html");
        }
        else if (request.find("lang=fr") != std::string::npos)
        {
            body = readFile("fr/text.html");
        }
        else
        {
            body = readFile("en/text.html");
        }
        makeHeader(response, okStatus, contentType);
        makeBody(response, body, true);
    }
    else if (request.find("HEAD") == 0)
    {
        makeHeader(response, okStatus, contentType);
        response += "\r\n";
    }
    else if (request.find("POST") == 0)
    {
        makeHeader(response, okStatus, "text/plain");
        response += "\r\nPOST request received\r\n";
        std::string::size_type bodyPos = request.find("\r\n\r\n");
        if (bodyPos != std::string::npos)
        {
            std::string body = request.substr(bodyPos + 4);
            std::cout << "POST body: " << body << std::endl;
        }
    }
    else if (request.find("PUT") == 0)
    {
        makeHeader(response, okStatus, "text/plain");
        response += "\r\nPUT request received\r\n";
    }
    else if (request.find("DELETE") == 0)
    {
        makeHeader(response, okStatus, "text/plain");
        response += "\r\nDELETE request received\r\n";
    }
    else if (request.find("TRACE") == 0)
    {
        makeHeader(response, okStatus, "message/http");
        makeBody(response, request, true);
    }
    else
    {
        makeHeader(response, notSupported, "text/plain");
        response += "\r\n";
    }

    strcpy(socket.buffer, response.c_str());
    socket.len = response.length();
    socket.send = SEND;
    socket.recv = IDLE;
}
