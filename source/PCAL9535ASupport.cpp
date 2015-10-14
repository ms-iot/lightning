// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "PCAL9535ASupport.h"
#include "ExpanderDefs.h"
#include "I2c.h"
#include "ErrorCodes.h"
#include "ArduinoCommon.h"

HRESULT PCAL9535ADevice::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR outPortBuf[1] = { 0x02 };         // Address of Output port, starts at Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;

    if (portBit > P1_7)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    //
    // Read what has been sent to the output port.
    //

    if (SUCCEEDED(hr))
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
        hr = transaction.queueWrite(outPortBuf, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Read the port contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Register a callback to set the desired bit state.
        hr = transaction.queueCallback([&dataBuf, bit, state]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | ((state & 0x01) << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Send the address of the output port to the I/O Expander chip.
        hr = transaction.queueWrite(outPortBuf, 1, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Send the desired state of the port bit to the I/O Expander chip.
        hr = transaction.queueWrite(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Actually perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

HRESULT PCAL9535ADevice::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR inPortBuf[1] = { 0x00 };         // Address of Input port, starts at Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;


    if (portBit > P1_7)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
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
        hr = transaction.queueWrite(inPortBuf, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Queue reading the port contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Perform the I2C transfers queued above.
        hr = transaction.execute(g_i2c.getController());
    }

    if (SUCCEEDED(hr))
    {
        // Pass back the bit value read from the port.
        state = (dataBuf[0] >> bit) & 0x01;
    }
    
    return hr;
}

HRESULT PCAL9535ADevice::SetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG direction)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR configAdrBuf[1] = { 0x06 };       // Address of config register, initially Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;
    UCHAR newData = 1;                      // Default to input direction

    if (portBit > P1_7)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    // Change to OUTPUT direction if that was specified.
    if (direction == DIRECTION_OUT)
    {
        newData = 0;
    }

    //
    // Read what is currently in the configuration register.
    //

    if (SUCCEEDED(hr))
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
        hr = transaction.queueWrite(configAdrBuf, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Read the configuration register contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    //
    // Create the new configuration register contents.
    //

    if (SUCCEEDED(hr))
    {
        // Register a callback to set the desired bit state.
        hr = transaction.queueCallback([&dataBuf, bit, newData]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | (newData << bit);
            return S_OK;
        }
        );
    }

    //
    // Write the new configuration register contents back to the I/O Expander chip.
    //

    if (SUCCEEDED(hr))
    {
        // Send the address of the configuration register to the I/O Expander chip.
        hr = transaction.queueWrite(configAdrBuf, 1, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Send the desired configuration to the I/O Expander chip.
        hr = transaction.queueWrite(dataBuf, sizeof(dataBuf));
    }

    //
    // Perform all the transfers specified above.
    //

    if (SUCCEEDED(hr))
    {
        // Actually perform the transfers.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

HRESULT PCAL9535ADevice::GetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG & direction)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR configAdrBuf[1] = { 0x06 };       // Address of config register, initially Port 0
    UCHAR dataBuf[1] = { 0 };
    ULONG bit = portBit;

    if (portBit > P1_7)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    //
    // Read what is currently in the configuration register.
    //

    if (SUCCEEDED(hr))
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
        hr = transaction.queueWrite(configAdrBuf, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Read the configuration register contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    //
    // Perform the transfers specified above.
    //
    if (SUCCEEDED(hr))
    {
        // Actually perform the transfers.
        hr = transaction.execute(g_i2c.getController());
    }

    //
    // Extract the desired bit from the port data.
    //
    if (SUCCEEDED(hr))
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
    
    return hr;
}
