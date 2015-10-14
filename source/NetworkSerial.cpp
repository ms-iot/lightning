// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.
// Licensed under the BSD 2-Clause License.
// See License.txt in the project root for license information.


#ifdef USE_NETWORKSERIAL
#include "arduino.h"
#include "NetworkSerial.h"
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN sizeof(uint8_t)

// Network serial listens for connections on a specified port
// By default, we chose 27015, but it can be any open port. 
// To specify a port, define NETWORK_SERIAL_PORT=<port> in your project
#ifndef NETWORKSERIAL_PORT
#define NETWORKSERIAL_PORT 27015
#endif

#define NETWORKSERIAL_PORT_S STRINGIFY_MACRO(NETWORKSERIAL_PORT) 

NetworkSerial::NetworkSerial()
    : _listenSocket(INVALID_SOCKET)
    , _clientSocket(INVALID_SOCKET)
{

}

void NetworkSerial::begin(unsigned long)
{
    WSADATA wsaData;
    struct addrinfo *result = NULL;
    struct addrinfo hints = { 0 };

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        goto Exit;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, NETWORKSERIAL_PORT_S, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        goto Exit;
    }

    // Create a SOCKET for connecting to server
    _listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        goto Exit;
    }

    // Setup the TCP listening socket
    iResult = bind(_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(_listenSocket);
        WSACleanup();
        goto Exit;
    }

    freeaddrinfo(result);

    iResult = listen(_listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(_listenSocket);
        WSACleanup();
        goto Exit;
    }

    // Accept a client socket
    _clientSocket = accept(_listenSocket, NULL, NULL);
    if (_clientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(_listenSocket);
        WSACleanup();
        goto Exit;
    }

    // No longer need server socket
    closesocket(_listenSocket);

Exit:
    ;
}

int NetworkSerial::available(void)
{
    unsigned char recvbuf;
    int recvbuflen = sizeof(recvbuf);
    int iResult = recv(_clientSocket, (char*)&recvbuf, recvbuflen, MSG_PEEK);
    return iResult > 0;
}

int NetworkSerial::read(void)
{
    unsigned char recvbuf;
    int recvbuflen = sizeof(recvbuf);
    int iResult = recv(_clientSocket, (char*)&recvbuf, recvbuflen, 0);
    if (iResult > 0)
    {
        return (int)recvbuf;
    }
    else
    {
        begin(0);
        int iResult = recv(_clientSocket, (char*)&recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            return (int)recvbuf;
        }
    }

    return 0;
}

size_t NetworkSerial::write(const uint8_t *buffer, size_t size)
{
    // Echo the buffer back to the sender
    int iSendResult = send(_clientSocket, (const char*)buffer, size, 0);
    if (iSendResult == SOCKET_ERROR)
    {
        begin(0);
        return send(_clientSocket, (const char*)buffer, size, 0);
    }
    else
    {
        return iSendResult;
    }
}


NetworkSerial Serial;
#endif
