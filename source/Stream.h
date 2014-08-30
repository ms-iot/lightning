// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once
#include "Print.h"

class Stream : public Print
{
public:
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    size_t write(uint8_t b)
    {
        return write(&b, 1);
    }

    virtual int available() = 0;
    virtual int read() = 0;

};