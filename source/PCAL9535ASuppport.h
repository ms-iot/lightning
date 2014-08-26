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

    // TODO: The following is not thread safe... must do read/modify/write under protection of I2C Lock.
    static BOOL SetBitState(UCHAR i2cAdr, UCHAR portBit, UCHAR state)
    {
        BOOL status = TRUE;
        DWORD error = ERROR_SUCCESS;
        I2cTransactionClass* transaction = new I2cTransactionClass;
        UCHAR outputPortBuffer[1] = { 0x02 };   // Address of Output port 0
        UCHAR dataBuffer[1] = { 0 };
        UCHAR bit = portBit;

        if (transaction == nullptr)
        {
            status = FALSE;
            error = ERROR_OUTOFMEMORY;
        }

        if (status && (portBit > P1_7))
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        //
        // Read what has been sent to the output port.
        //

        if (status)
        {
            // If the bit is on Port1, correct the output port address and bit number.
            if (portBit > P0_7)
            {
                outputPortBuffer[0] = 0x03;     // Address of Output port 1
                bit = bit - P1_0;               // Bit number on Output port 1
            }

            // Prepare to send the address of the output port to the I/O Expander chip.
            status = transaction->write(outputPortBuffer, 1);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Prepare to read the port contents from the I/O Expander chip.
            status = transaction->read(dataBuffer, sizeof(dataBuffer));
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Read the port.
            status = transaction->execute();
            if (!status) { error = GetLastError(); }
        }

        //
        // If the bit state has changed, write it to the Output port.
        //

        if (status)
        {
            // If the bit is desired in the other state:
            if (((dataBuffer[0] >> bit) & 0x01) != state)
            {
                // Flip the bit.
                dataBuffer[0] = dataBuffer[0] ^ (0x01 << bit);

                // Prepare to re-use the transaction object.
                transaction->reset();

                // Prepare to send the address of the output port to the I/O Expander chip.
                status = transaction->write(outputPortBuffer, 1);
                if (!status) { error = GetLastError(); }

                if (status)
                {
                    // Prepare to send the port contents to the I/O Expander chip.
                    status = transaction->write(dataBuffer, sizeof(dataBuffer));
                    if (!status) { error = GetLastError(); }
                }

                if (status)
                {
                    // Write the port
                    status = transaction->execute();
                    if (!status) { error = GetLastError(); }
                }
            }
        }

        delete transaction;
        transaction = nullptr;

        if (!status)
        {
            SetLastError(error);
        }
        return status;
    }

private:
};

#endif  // _PCAL9535A_SUPPORT_H_