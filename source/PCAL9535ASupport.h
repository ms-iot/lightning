// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _PCAL9535A_SUPPORT_H_
#define _PCAL9535A_SUPPORT_H_

#include <Windows.h>

class PCAL9535ADevice
{
public:
    PCAL9535ADevice()
    {
    }

    virtual ~PCAL9535ADevice()
    {
    }

    static BOOL SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state);

    static BOOL GetBitState(ULONG i2cAdr, ULONG portbit, ULONG & state);

    static BOOL SetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG direction);

    static BOOL GetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG & direction);

private:
};

#endif  // _PCAL9535A_SUPPORT_H_