// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _PCAL9535A_SUPPORT_H_
#define _PCAL9535A_SUPPORT_H_

#include <Windows.h>

#include "I2cController.h"

class PCAL9535ADevice
{
public:
    PCAL9535ADevice()
    {
    }

    virtual ~PCAL9535ADevice()
    {
    }

    static BOOL SetBitState(UCHAR i2cAdr, UCHAR bit, UCHAR state)
    {
        BOOL status = TRUE;
        DWORD error = ERROR_SUCCESS;


        if (!status)
        {
            SetLastError(error);
        }
        return status;
    }

private:
};

#endif  // _PCAL9535A_SUPPORT_H_