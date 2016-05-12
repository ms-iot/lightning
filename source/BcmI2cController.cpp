// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "I2c.h"
#include "BcmI2cController.h"
#include "BoardPins.h"
#include "I2cTransfer.h"
#include "HiResTimer.h"


// Initialize the specified I2C bus for use.
HRESULT BcmI2cControllerClass::begin(ULONG busNumber)
{
    m_busNumber = busNumber;

    // Map the I2C controller if it has not already been mapped.
    // This is done here so if it is going to fail, it will fail during the 
    // begin() call in the user's code rather than in transaction.execute().
    return mapIfNeeded();
}


// This method configures the pins to be used for this I2C bus.
HRESULT BcmI2cControllerClass::configurePins(ULONG sdaPin, ULONG sclPin)
{
    HRESULT hr = S_OK;

    // Record the pins this I2C controller is using.
    m_sdaPin = sdaPin;
    m_sclPin = sclPin;

    // Set the MUXes for external I2C use.
    hr = g_pins.verifyPinFunction(sclPin, FUNC_I2C, BoardPinsClass::LOCK_FUNCTION);

    if (SUCCEEDED(hr))
    {
        hr = g_pins.verifyPinFunction(sdaPin, FUNC_I2C, BoardPinsClass::LOCK_FUNCTION);
    }

    // If we did not succssfully configure the pins, return them to GPIO mode on a
    // best effort basis.
    if (FAILED(hr))
    {
        revertPinsToGpio();
    }

    return hr;
}

// Method to initialize the I2C Controller at the start of a transaction.
HRESULT BcmI2cControllerClass::_initializeForTransaction(ULONG slaveAddress, BOOL useHighSpeed)
{
    _C controlReg;
    _S statusReg;
    _DIV divReg;
    _A addressReg;
    _CLKT clktReg;


    // Disable the controller and controller interrupts.
    controlReg.ALL_BITS = 0;
    m_registers->C.ALL_BITS = controlReg.ALL_BITS;

    // Clear status bits that may be set from a previous transaction.
    statusReg.ALL_BITS = 0;
    statusReg.CLKT = 1;
    statusReg.ERR = 1;
    statusReg.DONE = 1;
    m_registers->S.ALL_BITS = statusReg.ALL_BITS;

    // Clear the RX and TX FIFOS.
    controlReg.CLEAR = 3;
    m_registers->C.ALL_BITS = controlReg.ALL_BITS;

    // Wait for the controller to go idle.
    while (m_registers->S.TA == 1);

    // Set the desired I2C Clock speed.
    if (useHighSpeed)
    {
        divReg.ALL_BITS = m_registers->DIV.ALL_BITS;
        divReg.ALL_BITS &= _DIV_USED_MASK;
        divReg.CDIV = CDIV_400KHZ;
        m_registers->DIV.ALL_BITS = divReg.ALL_BITS;
    }
    else
    {
        divReg.ALL_BITS = m_registers->DIV.ALL_BITS;
        divReg.ALL_BITS &= _DIV_USED_MASK;
        divReg.CDIV = CDIV_100KHZ;
        m_registers->DIV.ALL_BITS = divReg.ALL_BITS;
    }

    // Set the address of the slave this tranaction affects.
    addressReg.ALL_BITS = m_registers->A.ALL_BITS;
    addressReg.ALL_BITS &= _A_USED_MASK;
    addressReg.ADDR = slaveAddress & 0x7F;
    m_registers->A.ALL_BITS = addressReg.ALL_BITS;

    // Disable bus slave timeouts.
    clktReg.ALL_BITS = 0;
    m_registers->CLKT.ALL_BITS = clktReg.ALL_BITS;

    // Enable the controller.
    controlReg.ALL_BITS = 0;
    controlReg.I2CEN = 1;
    m_registers->C.ALL_BITS = controlReg.ALL_BITS;

    return S_OK;
}

// Method to map the I2C controller into this process' virtual address space.
HRESULT BcmI2cControllerClass::_mapController()
{
    HRESULT hr = S_OK;
    PVOID baseAddress = nullptr;
    PWCHAR deviceName = nullptr;


    if (SUCCEEDED(hr))
    {
        switch (m_busNumber)
        {
        case EXTERNAL_I2C_BUS:
            deviceName = pi2I2c1DeviceName;
            break;
        case SECOND_EXTERNAL_I2C_BUS:
            deviceName = pi2I2c0DeviceName;
            break;
        default:
            hr = DMAP_E_I2C_INVALID_BUS_NUMBER_SPECIFIED;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = GetControllerBaseAddress(deviceName,
            m_hController,
            baseAddress,
            FILE_SHARE_READ | FILE_SHARE_WRITE);
        if (SUCCEEDED(hr))
        {
            m_registers = (PI2C_CONTROLLER)baseAddress;
        }
    }

    return hr;
}

// Method to handle a set of contiguous transfers in a transaction.  The set of
// contiguous transfers ends at the end of the transaction, or at a transfer with 
// a callback, whichever occurs first.
HRESULT BcmI2cControllerClass::_performContiguousTransfers(I2cTransferClass* & pXfr)
{
    HRESULT hr = S_OK;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* tmpXfr = nullptr;


    if (pXfr == nullptr)
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }
    else
    {
        cmdXfr = pXfr;
    }

    // While we have more transfers to perform:
    if (SUCCEEDED(hr) && (cmdXfr != nullptr) && !pXfr->hasCallback())
    {
        //
        // Within a set of contigous transfers, we can have one or more of any of the 
        // following types of transfers: Read, Write, or Write-Restart-Read
        // Determine which type we are handling.
        //
        if (cmdXfr->transferIsRead())
        {
            // If the next transfer is a read, it must be a simple read.
            hr = _performReads(cmdXfr);
        }
        else
        {
            // If the next transfer is a write, determine if it has a following read.
            tmpXfr = cmdXfr->getNextTransfer();
            while ((tmpXfr != nullptr) && !tmpXfr->hasCallback() && !tmpXfr->transferIsRead() && !tmpXfr->preResart())
            {
                tmpXfr = tmpXfr->getNextTransfer();
            }

            if (tmpXfr == nullptr || tmpXfr->hasCallback() || (tmpXfr->preResart() && !tmpXfr->transferIsRead()))
            {
                // If the write ends with transaction end, or callback, or transfer with a 
                // pre-restart (that is not also a read transfer) it is a simple write.
                hr = _performWrites(cmdXfr);
            }
            else
            {
                // If the write is followed by a read, do Write-Restart-Read sequence.
                hr = _performWriteRead(cmdXfr);
            }
        }
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = cmdXfr;

    return hr;
}

// Perform one or more contiguous write transfers.
HRESULT BcmI2cControllerClass::_performWrites(I2cTransferClass* & pXfr)
{
    HRESULT hr = S_OK;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* tmpXfr = nullptr;
    LONG cmdsOutstanding = 0;
    UCHAR outByte;
    _S sReg;
    _C cReg;


    // Calculate the total number of bytes we will be writing in this set of transfers.
    // The set of transfers can end at end of transaction or callback, or a transfer that
    // specifies a pre-restart.  We know it is not ended by a read transfer because that 
    // would be Write-Restart-Read transfer type.
    cmdXfr = pXfr;
    tmpXfr = cmdXfr;
    if (tmpXfr != nullptr)
    {
        cmdsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }
    while ((tmpXfr != nullptr) && !tmpXfr->hasCallback() && !tmpXfr->preResart())
    {
        cmdsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }

    if (cmdsOutstanding > m_maxTransferBytes)
    {
        hr = DMAP_E_I2C_TRANSFER_LENGTH_OVER_MAX;
    }

    if (SUCCEEDED(hr))
    {
        // Prepare to access the cmd buffer.
        cmdXfr->resetCmd();

        // Tell the controller how many bytes we expect to write.
        m_registers->DLEN.ALL_BITS = cmdsOutstanding;

        // Prepare for the writes.
        sReg.ALL_BITS = 0;
        sReg.CLKT = 1;
        sReg.DONE = 1;
        sReg.ERR = 1;
        m_registers->S.ALL_BITS = sReg.ALL_BITS;

        // Slave address has already been set.
        cReg.ALL_BITS = m_registers->C.ALL_BITS;
        cReg.ALL_BITS &= _C_USED_MASK;
        cReg.READ = 0;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;

        // Start the transfer.
        cReg.ST = 1;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;
    }

    // While we have more bytes to write:
    while (SUCCEEDED(hr) && (cmdXfr != nullptr) && (cmdsOutstanding > 0))
    {
        while (SUCCEEDED(hr) && (cmdXfr->getNextCmd(outByte)))
        {
            // Wait for at least one empty space in the TX FIFO.
            while (SUCCEEDED(hr) && txFifoFull())
            {
                sReg.ALL_BITS = m_registers->S.ALL_BITS;
                if (sReg.ERR == 1)
                {
                    hr = E_FAIL;
                }
            }

            if (SUCCEEDED(hr))
            {
                // Write the byte.
                m_registers->FIFO.ALL_BITS = outByte;

                // Count the byte we sent.
                cmdsOutstanding--;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Get the next transfer in the transaction if there is one.
            cmdXfr = cmdXfr->getNextTransfer();
        }
    }

    if (SUCCEEDED(hr))
    {
        // Wait for the writes to complete.
        do
        {
            sReg.ALL_BITS = m_registers->S.ALL_BITS;
        } while (sReg.DONE == 0);
    }

    // Determine if an error occurred.
    if (SUCCEEDED(hr))
    {
        hr = _handleErrors();
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = cmdXfr;

    if (SUCCEEDED(hr))
    {
        // Check for some catch-all errors.
        if (cmdsOutstanding > 0)
        {
            hr = DMAP_E_I2C_OPERATION_INCOMPLETE;
        }
        else if (cmdsOutstanding < 0)
        {
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

// Perform one or more contiguous read transfers.
HRESULT BcmI2cControllerClass::_performReads(I2cTransferClass* & pXfr)
{
    HRESULT hr = S_OK;
    I2cTransferClass* tmpXfr = nullptr;
    I2cTransferClass* readXfr = nullptr;
    PUCHAR readPtr = nullptr;
    LONG cmdsOutstanding = 0;
    UCHAR inByte;
    _S sReg;
    _C cReg;

    // Calculate the total number of bytes we will be reading in this set of transfers.
    readXfr = pXfr;
    tmpXfr = readXfr;
    readXfr->resetCmd();
    // The set of transfers can end with transaction, callback, a write transfer, or
    // a transfer that specifies a pre-restart.
    if (tmpXfr != nullptr)
    {
        cmdsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }
    while ((tmpXfr != nullptr) && !tmpXfr->hasCallback() && tmpXfr->transferIsRead() && !tmpXfr->preResart())
    {
        cmdsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }
    // tmpXfr is left with the address of the terminating transfer (if any).
    
    if (cmdsOutstanding > m_maxTransferBytes)
    {
        hr = DMAP_E_I2C_TRANSFER_LENGTH_OVER_MAX;
    }

    if (SUCCEEDED(hr))
    {
        // Prepare to access the read buffer.
        readXfr->resetRead();
        readPtr = readXfr->getNextReadLocation();

        // Tell the controller how many bytes we expect to read.
        m_registers->DLEN.ALL_BITS = cmdsOutstanding;

        // Prepare for the reads.
        sReg.ALL_BITS = 0;
        sReg.CLKT = 1;
        sReg.DONE = 1;
        sReg.ERR = 1;
        m_registers->S.ALL_BITS = sReg.ALL_BITS;

        // Slave address has already been set.
        cReg.ALL_BITS = m_registers->C.ALL_BITS;
        cReg.ALL_BITS &= _C_USED_MASK;
        cReg.READ = 1;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;

        // Start the transfer.
        cReg.ST = 1;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;
    }

    // While we have more bytes to read:
    while (SUCCEEDED(hr) && (readXfr != nullptr) && (cmdsOutstanding > 0))
    {
        // Wait for at least one byte to be available in the RX FIFO.
        while (SUCCEEDED(hr) && rxFifoEmpty())
        {
            sReg.ALL_BITS = m_registers->S.ALL_BITS;
            if (sReg.ERR == 1)
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Read a byte from the I2C Controller.
            inByte = readByte();
            cmdsOutstanding--;

            // Store the byte if we have a place for it.
            if (readPtr != nullptr)
            {
                *readPtr = inByte;

                // Figure out where the next byte should go.
                readPtr = readXfr->getNextReadLocation();
                while ((readPtr == nullptr) && (readXfr->getNextTransfer() != nullptr))
                {
                    readXfr = readXfr->getNextTransfer();
                    readXfr->resetRead();
                    readPtr = readXfr->getNextReadLocation();
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Wait for the reads to complete.
        do
        {
            sReg.ALL_BITS = m_registers->S.ALL_BITS;
        }
        while (sReg.DONE == 0);
    }

    // Determine if an error occurred.
    if (SUCCEEDED(hr))
    {
        hr = _handleErrors();
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = tmpXfr;

    if (SUCCEEDED(hr))
    {
        // Check for some catch-all errors.
        if (cmdsOutstanding > 0)
        {
            hr = DMAP_E_I2C_OPERATION_INCOMPLETE;
        }
        else if (cmdsOutstanding < 0)
        {
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

// Perform a Write-Restart-Read sequence of transfers.
HRESULT BcmI2cControllerClass::_performWriteRead(I2cTransferClass* & pXfr)
{
    HRESULT hr = S_OK;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* tmpXfr = nullptr;
    PUCHAR readPtr = nullptr;
    LONG writesOutstanding = 0;
    LONG readsOutstanding = 0;
    UCHAR outByte;
    UCHAR inByte;
    _S sReg;
    _C cReg;


    cmdXfr = pXfr;
    
    // Calculate the number of bytes to write during the first part of the sequence.
    tmpXfr = cmdXfr;
    while ((tmpXfr != nullptr) && !tmpXfr->transferIsRead() && !tmpXfr->hasCallback())
    {
        writesOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }

    // Calculate the number of bytes to read during the 2nd part of the sequence.
    if (tmpXfr != nullptr)
    {
        readsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }
    while ((tmpXfr != nullptr) && tmpXfr->transferIsRead() && !tmpXfr->hasCallback() && !tmpXfr->preResart())
    {
        readsOutstanding += tmpXfr->getBufferSize();
        tmpXfr = tmpXfr->getNextTransfer();
    }
    // tmpXfr is left with the address of the terminating transfer, or nullptr if none.

    if ((writesOutstanding == 0) || (readsOutstanding == 0))
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if ((writesOutstanding > m_maxTransferBytes) || (readsOutstanding > m_maxTransferBytes))
    {
        hr = DMAP_E_I2C_TRANSFER_LENGTH_OVER_MAX;
    }

    //
    // Write bytes for the first part of the transfer sequence.
    //

    if (SUCCEEDED(hr))
    {
        // Tell the controller the number of bytes we expect to write.
        m_registers->DLEN.ALL_BITS = writesOutstanding;

        // Prepare for the write transfer.
        sReg.ALL_BITS = 0;
        sReg.CLKT = 1;
        sReg.DONE = 1;
        sReg.ERR = 1;
        m_registers->S.ALL_BITS = sReg.ALL_BITS;
        // Slave address has already been set.
        cReg.ALL_BITS = m_registers->C.ALL_BITS;
        cReg.ALL_BITS &= _C_USED_MASK;
        cReg.READ = 0;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;

        // Start the transfer.
        cReg.ST = 1;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;

        // Wait for the transfer to be active.
        while (m_registers->S.TA == 0);

        // While we have more bytes to write:
        while (SUCCEEDED(hr) && (cmdXfr != nullptr) && (writesOutstanding > 0))
        {
            while (SUCCEEDED(hr) && (cmdXfr->getNextCmd(outByte)))
            {
                // If this is not the last byte to write:
                if (writesOutstanding > 1)
                {
                    // Wait for at least one empty space in the TX FIFO.
                    while (SUCCEEDED(hr) && txFifoFull())
                    {
                        sReg.ALL_BITS = m_registers->S.ALL_BITS;
                        if (sReg.ERR == 1)
                        {
                            hr = E_FAIL;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        // Write the byte.
                        m_registers->FIFO.ALL_BITS = outByte;
                    }
                }

                if (SUCCEEDED(hr))
                {
                    // Count the byte as sent.
                    writesOutstanding--;
                }
            }

            if (SUCCEEDED(hr))
            {
                // Get the next transfer in the transaction, there should be one.
                cmdXfr = cmdXfr->getNextTransfer();
            }
        }
    }
    // At this point, on success, outByte contains the last byte to write and cmdXfr
    // has the addresss of the first read transfer that follows the writes.

    //
    // Read bytes for the second, final part of the sequence.
    //

    if (SUCCEEDED(hr))
    {
        // Tell the controller the number of bytes we expect to read.
        m_registers->DLEN.ALL_BITS = readsOutstanding;

        // Prepare and start the read transfer.
        cReg.ALL_BITS = m_registers->C.ALL_BITS;
        cReg.ALL_BITS &= _C_USED_MASK;
        cReg.READ = 1;
        cReg.ST = 1;
        m_registers->C.ALL_BITS = cReg.ALL_BITS;

        // Wait for at least one empty space in the TX FIFO.
        while (SUCCEEDED(hr) && txFifoFull())
        {
            sReg.ALL_BITS = m_registers->S.ALL_BITS;
            if (sReg.ERR == 1)
            {
                hr = E_FAIL;
            }
        }

        // Write the last byte so the write phase completes.
        if (SUCCEEDED(hr))
        {
            // Write the byte.
            m_registers->FIFO.ALL_BITS = outByte;

            // Indicate the current transfer is the first to read into.
            cmdXfr->resetRead();
            readPtr = cmdXfr->getNextReadLocation();

            // Wait for the controller to enter a read state.
            do
            {
                sReg.ALL_BITS = m_registers->S.ALL_BITS;
            }
            while (sReg.TA == 1);

            // Clear the DONE status for cleanliness.
            sReg.ALL_BITS = 0;
            sReg.DONE = 1;
            m_registers->S.ALL_BITS = sReg.ALL_BITS;
        }

        while (SUCCEEDED(hr) && (readsOutstanding > 0))
        {
            // Wait for at least one byte to be available in the RX FIFO.
            while (SUCCEEDED(hr) && rxFifoEmpty())
            {
                sReg.ALL_BITS = m_registers->S.ALL_BITS;
                if (sReg.ERR == 1)
                {
                    hr = E_FAIL;
                }
            }

            if (SUCCEEDED(hr))
            {
                // Read a byte from the I2C Controller.
                inByte = readByte();
                readsOutstanding--;

                // Store the byte if we have a place for it.
                if (readPtr != nullptr)
                {
                    *readPtr = inByte;

                    // Figure out where the next byte should go.
                    readPtr = cmdXfr->getNextReadLocation();
                    while ((readPtr == nullptr) && (cmdXfr->getNextTransfer() != nullptr))
                    {
                        cmdXfr = cmdXfr->getNextTransfer();
                        cmdXfr->resetRead();
                        readPtr = cmdXfr->getNextReadLocation();
                    }
                }
            }
        }
    }

    // Determine if an error occured on this transaction.
    if (SUCCEEDED(hr))
    {
        hr = _handleErrors();
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = tmpXfr;

    return hr;
}

