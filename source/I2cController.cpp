// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "I2cController.h"

//
// I2cControllerClass methods.
//

// This method is used to prepare to use the I2C bus to communicate with devices
// that are external to the board.
BOOL I2cControllerClass::beginExternal()
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;


    // Set the MUXes for external I2C use.
    status = _setPinI2c(PIN_I2C_CLK);
    if (!status) { status = GetLastError(); }

    if (status)
    {
        status = _setPinI2c(PIN_I2C_DAT);
        if (!status) { status = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

// This method returns the esternal I2C bus pins to their default configurations.
void I2cControllerClass::endExternal()
{
    // Set the pns used for I2C back to Digital inputs, on a best effort basis.
    _setPinFunction(PIN_I2C_DAT, FUNC_DIO);
    _setPinMode(PIN_I2C_DAT, INPUT, false);
    _setPinFunction(PIN_I2C_CLK, FUNC_DIO);
    _setPinMode(PIN_I2C_CLK, INPUT, false);
}

// This method maps the I2C controller if needed.
inline BOOL I2cControllerClass::mapIfNeeded()
{
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        return _mapController();
    }
    else
    {
        return TRUE;
    }
}

// Method to map the I2C controller into this process' virtual address space.
BOOL I2cControllerClass::_mapController()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    PVOID baseAddress = nullptr;

    status = GetControllerBaseAddress(dmapI2cDeviceName,
        m_hController,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }
    else
    {
        m_controller = (PI2C_CONTROLLER)baseAddress;
    }

    if (!status) { SetLastError(error); }
    return status;
}

//
// I2cTransferClass methods.
//

// Gets the next command/write byte.  Returns FALSE if there is none.
inline BOOL I2cTransferClass::getNextCmd(UCHAR & next)
{
    if (m_nextCmd >= m_bufBytes)
    {
        return FALSE;
    }
    else
    {
        next = m_pBuffer[m_nextCmd];
        m_nextCmd++;
        if (m_nextCmd == m_bufBytes)
        {
            m_lastCmdFetched = TRUE;
        }
        return TRUE;
    }
}

// Return the next available location in the read buffer, or nullptr
// if the read buffer is full.
inline PUCHAR I2cTransferClass::getNextReadLocation()
{
    if (!m_isRead || (m_nextRead >= m_bufBytes))
    {
        return nullptr;
    }
    else
    {
        PUCHAR nextRead = &(m_pBuffer[m_nextRead]);
        m_nextRead++;
        return nextRead;
    }
}

//
// I2cTransactionClass methods.
//

// Prepare this transaction for re-use.
// Any previously set slave address is not affected by this method.
void I2cTransactionClass::reset()
{
    I2cTransferClass* pCurrent = m_pFirstXfr;
    I2cTransferClass* pNext = nullptr;

    // Delete each transfer entry in the transfer queue.
    while (pCurrent != nullptr)
    {
        pNext = pCurrent->getNextTransfer();
        delete pCurrent;
        pCurrent = pNext;
    }
    m_pFirstXfr = nullptr;
    m_pXfrQueueTail = nullptr;
    m_cmdsOutstanding = 0;
    m_readsOutstanding = 0;

    m_readWaitCount = 0;
}

// Sets the 7-bit address of the slave for this tranaction.
BOOL I2cTransactionClass::setAddress(UCHAR slaveAdr)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;


    // Verify we successfully create the I2C Controller Lock.
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        status = FALSE;
        error = ERROR_CREATE_FAILED;
    }

    if (status && ((slaveAdr < 0x08) || (slaveAdr >= 0x77)))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        m_slaveAddress = slaveAdr;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Add a write transfer to the transaction.
BOOL I2cTransactionClass::write(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nullptr;


    // Sanity check the buffer and size parameters.
    if ((buffer == nullptr) || (bufferBytes == 0))
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Allocate a transfer object.
        pXfr = new I2cTransferClass;

        if (pXfr == 0)
        {
            status = FALSE;
            error = ERROR_OUTOFMEMORY;
        }
    }

    if (status)
    {
        // Mark transfer to start with a restart if that was requested.
        if (preRestart)
        {
            pXfr->markPreRestart();
        }

        // Fill in the transfer object (default is Write Transfer).
        pXfr->setBuffer(buffer, bufferBytes);

        // Queue the transfer as part of this transaction.
        _queueTransfer(pXfr);

        // Add the write byte count to the transaction outstanding command count.
        m_cmdsOutstanding = m_cmdsOutstanding + bufferBytes;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Add a read transfer to the transaction.
BOOL I2cTransactionClass::read(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nullptr;


    // Sanity check the buffer and size parameters.
    if ((buffer == nullptr) || (bufferBytes == 0))
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Allocate a transfer object.
        pXfr = new I2cTransferClass;

        if (pXfr == 0)
        {
            status = FALSE;
            error = ERROR_OUTOFMEMORY;
        }
    }

    if (status)
    {
        // Mark transfer to start with a restart if that was requested.
        if (preRestart)
        {
            pXfr->markPreRestart();
        }

        // Fill in the transfer object.
        pXfr->setBuffer(buffer, bufferBytes);
        pXfr->markReadTransfer();

        // Queue the transfer as part of this transaction.
        _queueTransfer(pXfr);

        // Add the read byte count to the transaction outstanding command and read counts.
        m_cmdsOutstanding = m_cmdsOutstanding + bufferBytes;
        m_readsOutstanding = m_readsOutstanding + bufferBytes;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to perform the transfers associated with this transaction.
BOOL I2cTransactionClass::execute()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nullptr;
    I2cTransferClass* pReadXfr = nullptr;
    DWORD lockResult = 0;
    BOOL haveLock = FALSE;


    // TODO: Verify transaction parameters are set up properly (?)

    // Verify we successfully created the I2C Controller Lock.
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        status = FALSE;
        error = ERROR_CREATE_FAILED;
    }

    // Claim the I2C controller.
    lockResult = WaitForSingleObject(m_hI2cLock, 5000);
    if ((lockResult == WAIT_OBJECT_0) || (lockResult == WAIT_ABANDONED))
    {
        haveLock = TRUE;
    }
    else if (lockResult == WAIT_TIMEOUT)
    {
        status = FALSE;
        error = ERROR_TIMEOUT;
    }
    else
    {
        status = FALSE;
        error = GetLastError();
    }

    // Initialize the controller.
    status = _initializeI2cForTransaction();
    if (!status) { error = GetLastError(); }

    // Process each transfer on the queue.
    if (status)
    {
        status = _processTransfers();
        if (!status) { error = GetLastError(); }

        // Shut down the controller.
        _shutDownI2cAfterTransaction();
    }

    // Release this transaction's claim on the controller.
    if (haveLock)
    {
        status = ReleaseMutex(m_hI2cLock);
        if (!status) { error = GetLastError(); }
        haveLock = FALSE;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to queue a transfer as part of this transaction.
void I2cTransactionClass::_queueTransfer(I2cTransferClass* pXfr)
{
    // If the transfer queue is empty:
    if (m_pXfrQueueTail == nullptr)
    {
        // Add this transfer as the first entry in the queue.
        m_pFirstXfr = pXfr;
        m_pXfrQueueTail = pXfr;
    }

    // If there is at least one other transfer in the queue:
    else
    {
        // Add this entry to the tail of the queue.
        m_pXfrQueueTail->chainNextTransfer(pXfr);
        m_pXfrQueueTail = pXfr;
    }
}

// Method to initialize the I2C Controller at the start of a transaction.
BOOL I2cTransactionClass::_initializeI2cForTransaction()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;


    // Get the I2C Controller mapped if it is not mapped yet.
    status = g_i2c.mapIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        // Make sure the I2C controller is disabled.
        g_i2c.disableController();

        // Set the desired I2C Clock speed.
        g_i2c.setStandardSpeed();

        // Allow bus restarts.
        g_i2c.enableRestartSupport();

        // Set the address of the slave this tranaction affects.
        g_i2c.set7bitAddressing();
        g_i2c.setAddress(m_slaveAddress);

        // Mask all interrupts.
        g_i2c.maskAllInterrupts();

        // Clear any outstanding interrupts.
        g_i2c.clearAllInterrupts();

        // We know that when the I2C Controller is disabled the RX and TX
        // FIFOs are cleared, so we don't need to flush them here.

        // Enable the controller.
        g_i2c.enableController();
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to process each transfer in this transaction.
BOOL I2cTransactionClass::_processTransfers()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* readXfr = nullptr;
    PUCHAR readPtr = nullptr;
    BOOL restart = FALSE;
    ULONG cmdDat;
    ULONG readWaitCount;
    UCHAR writeByte;
    UCHAR readByte;


    // For each transfer in the queue:
    cmdXfr = m_pFirstXfr;
    if ((cmdXfr != nullptr) && (cmdXfr->preResart()))
    {
        restart = TRUE;
    }
    while ((m_cmdsOutstanding > 0) && (cmdXfr != nullptr))
    {
        // If this is the first read transfer:
        if ((readXfr == nullptr) && cmdXfr->transferIsRead())
        {
            // Indicate this is the transfer to read into.
            readXfr = cmdXfr;
            readXfr->resetRead();
            readPtr = readXfr->getNextReadLocation();
        }

        // Prepare to access the cmd buffer.
        cmdXfr->resetCmd();

        // For each byte in the transfer:
        while (cmdXfr->getNextCmd(writeByte))
        {
            // Wait for at least one empty space in the TX FIFO.
            while (g_i2c.txFifoFull());

            // Issue the command.
            if (cmdXfr->transferIsRead())
            {
                cmdDat = 0x100;             // Build read command (data is ignored)
            }
            else
            {
                cmdDat = writeByte;         // Build write command with data byte
            }

            // If restart has been requested, signal a pre-RESTART.
            if (restart)
            {
                cmdDat = cmdDat | (1 << 10);
                restart = FALSE;            // Only one RESTART allowed per transfer
            }

            // If this is the last command in the last buffer, signal a STOP.
            if (cmdXfr->lastCmdFetched() && (cmdXfr->getNextTransfer() == nullptr))
            {
                cmdDat = cmdDat | (1 << 9);
            }

            g_i2c.issueCmd(cmdDat);
            m_cmdsOutstanding--;

            // Pull any available bytes out of the receive FIFO.
            while (g_i2c.rxFifoNotEmtpy())
            {
                // Read a byte from the I2C Controller.
                readByte = g_i2c.readByte();
                m_readsOutstanding--;

                // Store the byte if we have a place for it.
                if (readPtr != nullptr)
                {
                    *readPtr = readByte;

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
        cmdXfr = cmdXfr->getNextTransfer();
    }

    // Complete any outstanding reads.
    readWaitCount = 0;
    while (status && (m_readsOutstanding > 0))
    {
        // Pull any available bytes out of the receive FIFO.
        while (g_i2c.rxFifoNotEmtpy())
        {
            // Read a byte from the I2C Controller.
            readByte = g_i2c.readByte();
            m_readsOutstanding--;

            // Store the byte if we have a place for it.
            if (readPtr != nullptr)
            {
                *readPtr = readByte;

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

        if (m_readsOutstanding > 0)
        {
            if (readWaitCount < 50)
            {
                Sleep(20);
                readWaitCount++;
            }
            else
            {
                status = FALSE;
                error = ERROR_RECEIVE_PARTIAL;
            }
        }
    }

    m_readWaitCount = readWaitCount;

    if (status)
    {
        if (m_cmdsOutstanding > 0)
        {
            status = FALSE;
            error = ERROR_MORE_DATA;
        }
        else if (m_readsOutstanding < 0)
        {
            status = FALSE;
            error = ERROR_DATA_NOT_ACCEPTED;
        }
        else if (m_cmdsOutstanding < 0)
        {
            status = FALSE;
            error = ERROR_TOO_MANY_CMDS;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to shut down the I2C Controller after a transaction is done with it.
BOOL I2cTransactionClass::_shutDownI2cAfterTransaction()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    ULONG loopCount = 0;


    // Wait for the TX FIFO to go empty.
    while (!g_i2c.txFifoEmpty())
    {
        // Voluntarily give up the CPU if another thread is waiting.
        Sleep(0);
    }

    // Disable the controller.
    g_i2c.disableController();

    // Wait for it to go inactive.
    while (g_i2c.isActive() && (loopCount < 5))
    {
        Sleep(1);
        loopCount++;
    }

    if (!status) { SetLastError(error); }
    return status;
}
