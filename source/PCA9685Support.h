// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _PCA9685_SUPPORT_H_
#define _PCA9685_SUPPORT_H_

#include <Windows.h>

class PCA9685Device
{
public:
    PCA9685Device()
    {
    }

    virtual ~PCA9685Device()
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

#endif  // _PCA9685_SUPPORT_H_