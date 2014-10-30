// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "PCAL9535ASupport.h"
#include "ExpanderDefs.h"
#include "I2cController.h"

BOOL PCAL9535ADevice::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR outPortBuf[1] = { 0x02 };         // Address of Output port, starts at Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;


    if (portBit > P1_7)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    //
    // Read what has been sent to the output port.
    //

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // If the bit is on Port1, correct the output port address and bit number.
        if (portBit > P0_7)
        {
            outPortBuf[0]++;
            bit = bit & 0x07;               // Bit number on Output port 1
        }

        // Send the address of the output port to the I/O Expander chip.
        status = transaction.queueWrite(outPortBuf, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Read the port contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Register a callback to set the desired bit state.
        status = transaction.queueCallback([&dataBuf, bit, state]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | ((state & 0x01) << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Send the address of the output port to the I/O Expander chip.
        status = transaction.queueWrite(outPortBuf, 1, TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Send the desired state of the port bit to the I/O Expander chip.
        status = transaction.queueWrite(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Actually perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

BOOL PCAL9535ADevice::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR inPortBuf[1] = { 0x00 };         // Address of Input port, starts at Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;


    if (portBit > P1_7)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // If the bit is on Port1, correct the input port address and bit number.
        if (portBit > P0_7)
        {
            inPortBuf[0]++;
            bit = bit & 0x07;               // Bit number on Output port 1
        }

        // Queue sending the address of the input port to the I/O Expander chip.
        status = transaction.queueWrite(inPortBuf, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue reading the port contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Perform the I2C transfers queued above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Pass back the bit value read from the port.
        state = (dataBuf[0] >> bit) & 0x01;
    }

    if (!status) { SetLastError(error); }
    return status;
}

BOOL PCAL9535ADevice::SetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG direction)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR configAdrBuf[1] = { 0x06 };       // Address of config register, initially Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;
    UCHAR newData = 1;                      // Default to input direction


    if (portBit > P1_7)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    // Change to OUTPUT direction if that was specified.
    if (direction == DIRECTION_OUT)
    {
        newData = 0;
    }

    //
    // Read what is currently in the configuration register.
    //

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // If the bit is on Port1, correct the config register address and bit number.
        if (portBit > P0_7)
        {
            configAdrBuf[0]++;
            bit = bit & 0x07;               // Bit number on Output port 1
        }

        // Send the address of the configuration register to the I/O Expander chip.
        status = transaction.queueWrite(configAdrBuf, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Read the configuration register contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    //
    // Create the new configuration register contents.
    //

    if (status)
    {
        // Register a callback to set the desired bit state.
        status = transaction.queueCallback([&dataBuf, bit, newData]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | (newData << bit);
            return TRUE;
        }
        );
    }

    //
    // Write the new configuration register contents back to the I/O Expander chip.
    //

    if (status)
    {
        // Send the address of the configuration register to the I/O Expander chip.
        status = transaction.queueWrite(configAdrBuf, 1, TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Send the desired configuration to the I/O Expander chip.
        status = transaction.queueWrite(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    //
    // Perform all the transfers specified above.
    //

    if (status)
    {
        // Actually perform the transfers.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

BOOL PCAL9535ADevice::GetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG & direction)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR configAdrBuf[1] = { 0x06 };       // Address of config register, initially Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;


    if (portBit > P1_7)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    //
    // Read what is currently in the configuration register.
    //

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // If the bit is on Port1, correct the config register address and bit number.
        if (portBit > P0_7)
        {
            configAdrBuf[0]++;
            bit = bit & 0x07;               // Bit number on Output port 1
        }

        // Send the address of the configuration register to the I/O Expander chip.
        status = transaction.queueWrite(configAdrBuf, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Read the configuration register contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    //
    // Perform the transfers specified above.
    //
    if (status)
    {
        // Actually perform the transfers.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    //
    // Extract the desired bit from the port data.
    //
    if (status)
    {
        if (((dataBuf[0] >> bit) & 0x01) == 0)
        {
            direction = DIRECTION_OUT;
        }
        else
        {
            direction = DIRECTION_IN;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}
