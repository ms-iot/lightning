// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "BtI2cController.h"
#include "HiResTimer.h"

#include "BoardPins.h"

//
// BtI2cControllerClass methods.
//

// Initialize the specified I2C bus for use.
HRESULT BtI2cControllerClass::begin(ULONG busNumber)
{
    return S_OK;
}


// This method configures the pins to be used for this I2C bus.
HRESULT BtI2cControllerClass::configurePins(ULONG sdaPin, ULONG sclPin)
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
HRESULT BtI2cControllerClass::_initializeForTransaction(ULONG slaveAddress, BOOL useHighSpeed)
{
    ULONGLONG waitStartTicks = 0;
    BoardPinsClass::BOARD_TYPE board;
    _IC_CON icConReg;

    // If we need to initialize, or re-initialize, the I2C Controller:
    if (!isInitialized() || (m_registers->IC_TAR.IC_TAR != slaveAddress))
    {
        // Disable the I2C controller.  This also clears the FIFOs.
        m_registers->IC_ENABLE.ENABLE = 0;

        // Wait for the controller to go disabled, but only for 100 mS.
        // It can latch in a mode in which it does not go disabled, but appears 
        // to come out of this state when used again.
        waitStartTicks = GetTickCount64();
        while ((m_registers->IC_ENABLE_STATUS.IC_EN == 1) && ((GetTickCount64() - waitStartTicks) < 100))
        {
            Sleep(0);       // Give the CPU to any thread that is waiting
        }

        // Set the desired I2C Clock speed.
        g_pins.getBoardType(board);
        if ((board == BoardPinsClass::BOARD_TYPE::MBM_BARE) ||
            (board == BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE))
        {
            if (useHighSpeed)
            {
                m_registers->IC_FS_SCL_HCNT.IC_FS_SCL_HCNT = 0x3C;
                m_registers->IC_FS_SCL_LCNT.IC_FS_SCL_LCNT = 0x82;
                m_registers->IC_CON.SPEED = 2;
            }
            else
            {
                m_registers->IC_SS_SCL_HCNT.IC_SS_SCL_HCNT = 0x190;
                m_registers->IC_SS_SCL_LCNT.IC_SS_SCL_LCNT = 0x1D6;
                m_registers->IC_CON.SPEED = 1;
            }
        }
        else
        {
            if (useHighSpeed)
            {
                m_registers->IC_FS_SCL_HCNT.IC_FS_SCL_HCNT = 0x14;
                m_registers->IC_FS_SCL_LCNT.IC_FS_SCL_LCNT = 0x2E;
                m_registers->IC_CON.SPEED = 2;
            }
            else
            {
                m_registers->IC_SS_SCL_HCNT.IC_SS_SCL_HCNT = 0x92;
                m_registers->IC_SS_SCL_LCNT.IC_SS_SCL_LCNT = 0xAB;
                m_registers->IC_CON.SPEED = 1;
            }
        }

        // Allow bus restarts.
        m_registers->IC_CON.IC_RESTART_EN = 1;

        // Set 7-bit addressing.
        icConReg.ALL_BITS = m_registers->IC_CON.ALL_BITS;
        icConReg.IC_10BITADDR_MASTER = 0;
        m_registers->IC_CON.ALL_BITS = icConReg.ALL_BITS;

        // Set the address of the slave this tranaction affects.
        // All bits but the 7-bit address are intentionally cleared here.  This is needed
        // for Bay Trail, which supports additional bits (all of which we want clear).
        m_registers->IC_TAR.ALL_BITS = (slaveAddress & 0x7F);

        // Mask all interrupts.
        m_registers->IC_INTR_MASK.ALL_BITS = 0;

        // Clear any outstanding interrupts.
        ULONG dummy = m_registers->IC_CLR_INTR.ALL_BITS;

        // Enable the controller.
        m_registers->IC_ENABLE.ENABLE = 1;

        // Indicate the I2C Controller is now initialized.
        setInitialized();

    } // End - if (!isInitialized() || (getAddress() != m_slaveAddress))

    return S_OK;
}

// Method to map the I2C controller into this process' virtual address space.
HRESULT BtI2cControllerClass::_mapController()
{
    HRESULT hr = S_OK;
    PVOID baseAddress = nullptr;
    BoardPinsClass::BOARD_TYPE board;
    PWCHAR deviceName = nullptr;

    hr = g_pins.getBoardType(board);
 
    if (SUCCEEDED(hr))
    {
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
        case BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE:
            deviceName = mbmI2cDeviceName;
            break;
        default:
            hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
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

HRESULT BtI2cControllerClass::_performContiguousTransfers(I2cTransferClass* & pXfr)
{
    ULONGLONG startWaitTicks = 0;
    ULONGLONG currentTicks = 0;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* readXfr = nullptr;
    PUCHAR readPtr = nullptr;
    HRESULT hr = S_OK;
    BOOL restart = FALSE;
    ULONG cmdDat;
    LONG cmdsOutstanding = 0;
    LONG readsOutstanding = 0;
    UCHAR outByte;
    UCHAR inByte;


    if (pXfr == nullptr)
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the command and read counts for the current sequence of 
        // contiguous transfers in this transaction.
        hr = calculateCurrentCounts(pXfr, cmdsOutstanding, readsOutstanding);
    }

    // For each transfer in this section of the transaction:
    cmdXfr = pXfr;
    while (SUCCEEDED(hr) && (cmdsOutstanding > 0) && (cmdXfr != nullptr))
    {
        // If this is the first read transfer in this sequence of transfers:
        if ((readXfr == nullptr) && cmdXfr->transferIsRead())
        {
            // Indicate this is the transfer to read into.
            readXfr = cmdXfr;
            readXfr->resetRead();
            readPtr = readXfr->getNextReadLocation();
        }

        // Prepare to access the cmd buffer.
        cmdXfr->resetCmd();

        // Signal a pre-restart if this transfer is marked for one.
        if (cmdXfr->preResart())
        {
            restart = TRUE;
        }

        // For each byte in the transfer:
        while (SUCCEEDED(hr) && (cmdXfr->getNextCmd(outByte)))
        {
            // Wait for at least one empty space in the TX FIFO.
            while (txFifoFull());

            // Issue the command.
            if (cmdXfr->transferIsRead())
            {
                cmdDat = 0x100;             // Build read command (data is ignored)
            }
            else
            {
                cmdDat = outByte;           // Build write command with data byte
            }

            // If restart has been requested, signal a pre-RESTART.
            if (restart)
            {
                cmdDat = cmdDat | (1 << 10);
                restart = FALSE;            // Only want to RESTART on first command of transfer
            }

            // If this is the last command before the end of the transaction or
            // before a callback, signal a STOP.
            if (cmdsOutstanding == 1)
            {
                cmdDat = cmdDat | (1 << 9);
            }

            // Issue the command.
            m_registers->IC_DATA_CMD.ALL_BITS = cmdDat;
            cmdsOutstanding--;

            hr = _handleErrors();

            // Pull any available bytes out of the receive FIFO.
            while (SUCCEEDED(hr) && rxFifoNotEmtpy())
            {
                // Read a byte from the I2C Controller.
                inByte = readByte();
                readsOutstanding--;

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
            cmdXfr = cmdXfr->getNextTransfer();
        }
    }

    // Complete any outstanding reads and wait for the TX FIFO to empty.
    startWaitTicks = GetTickCount64();
    currentTicks = startWaitTicks;
    while (SUCCEEDED(hr) && ((readsOutstanding > 0) || !txFifoEmpty()) && !errorOccurred())
    {
        // Pull any available bytes out of the receive FIFO.
        while (rxFifoNotEmtpy())
        {
            // Read a byte from the I2C Controller.
            inByte = readByte();
            readsOutstanding--;

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

        // Wait up to to 100 milliseconds for transfers to happen.
        if (readsOutstanding > 0)
        {
            currentTicks = GetTickCount64();
            if (((currentTicks - startWaitTicks) > 100) && rxFifoEmpty())
            {
                hr = DMAP_E_I2C_READ_INCOMPLETE;
            }
        }
    }

    // Determine if an error occured on this transaction.
    if (SUCCEEDED(hr))
    {
        hr = _handleErrors();
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = cmdXfr;

    // Record the read wait count for debugging purposes.
    if ((currentTicks - startWaitTicks) > m_maxWaitTicks)
    {
        m_maxWaitTicks = (ULONG)(currentTicks - startWaitTicks);
    }

    if (SUCCEEDED(hr))
    {
        // Check for some catch-all errors.
        if (cmdsOutstanding > 0)
        {
            hr = DMAP_E_I2C_OPERATION_INCOMPLETE;
        }
        else if (readsOutstanding < 0)
        {
            hr = DMAP_E_I2C_EXTRA_DATA_RECEIVED;
        }
        else if (cmdsOutstanding < 0)
        {
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

