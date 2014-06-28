// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once
#include "Stream.h"

class NetworkSerial : public Stream
{
private:
    SOCKET _listenSocket = INVALID_SOCKET;
    SOCKET _clientSocket = INVALID_SOCKET;

public:
    NetworkSerial();
    void begin(unsigned long);
    virtual int available(void);
    virtual int read(void);
    virtual size_t write(const uint8_t *buffer, size_t size);
};

extern NetworkSerial Serial;