// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "I2cController.h"
#include "HiResTimer.h"

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
    status = g_pins.verifyPinFunction(PIN_I2C_CLK, FUNC_I2C, GalileoPinsClass::LOCK_FUNCTION);
    if (!status) { status = GetLastError(); }

    if (status)
    {
        status = g_pins.verifyPinFunction(PIN_I2C_DAT, FUNC_I2C, GalileoPinsClass::LOCK_FUNCTION);
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
    g_pins.verifyPinFunction(PIN_I2C_DAT, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION);
    g_pins.setPinMode(PIN_I2C_DAT, DIRECTION_IN, false);
    g_pins.verifyPinFunction(PIN_I2C_CLK, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION);
    g_pins.setPinMode(PIN_I2C_CLK, DIRECTION_IN, false);
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
    m_maxWaitTicks = 0;
    m_abort = FALSE;
    m_error = SUCCESS;
    m_isIncomplete = FALSE;
}

// Sets the 7-bit address of the slave for this tranaction.
BOOL I2cTransactionClass::setAddress(ULONG slaveAdr)
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
BOOL I2cTransactionClass::queueWrite(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
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

        // Indicate this transaction has at least one incomplete transfer.
        m_isIncomplete = TRUE;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Add a read transfer to the transaction.
BOOL I2cTransactionClass::queueRead(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
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

        // Indicate this transaction has at least one incomplete transfer.
        m_isIncomplete = TRUE;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to queue a callback routine at the current point in the transaction.
BOOL I2cTransactionClass::queueCallback(std::function<BOOL()> callBack)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nullptr;

    if (callBack == nullptr)
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
        // Associate the callback with the transfer.
        status = pXfr->setCallback(callBack);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue the transfer as part of this transaction.
        _queueTransfer(pXfr);

        // Indicate this transaction has at least one incomplete "transfer."
        m_isIncomplete = TRUE;
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


    // Verify we successfully created the I2C Controller Lock.
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        status = FALSE;
        error = ERROR_CREATE_FAILED;
    }

    if (status)
    {
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
    }

    if (status)
    {
        // Initialize the controller.
        status = _initializeI2cForTransaction();
        if (!status) { error = GetLastError(); }
    }

    // Process each transfer on the queue.
    if (status)
    {
        status = _processTransfers();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Shut down the controller.
        status = _shutDownI2cAfterTransaction();
        if (!status) { error = GetLastError(); }
    }

    // Release this transaction's claim on the controller.
    if (haveLock)
    {
        ReleaseMutex(m_hI2cLock);
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
    ULONGLONG waitStartTicks = 0;


    // Get the I2C Controller mapped if it is not mapped yet.
    status = g_i2c.mapIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status && (!g_i2c.isInitialized() || (g_i2c.getAddress() != m_slaveAddress)))
    {
        // Make sure the I2C controller is disabled.
        g_i2c.disableController();

        // Wait for the controller to go disabled, but only for 100 mS.
        // It can latch in a mode in which it does not go disabled, but appears 
        // to come out of this state when used again.
        waitStartTicks = GetTickCount64();
        while (g_i2c.isEnabled() && ((GetTickCount64() - waitStartTicks) < 100))
        {
            Sleep(0);       // Give the CPU to any thread that is waiting
        }

        // Set the desired I2C Clock speed.
        if (m_useHighSpeed)
        {
            g_i2c.setFastSpeed();
        }
        else
        {
            g_i2c.setStandardSpeed();
        }

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

        // Indicate the I2C Controller is now initialized.
        g_i2c.setInitialized();
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to process the transfers in this transaction.
BOOL I2cTransactionClass::_processTransfers()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nullptr;

    // Clear out any data from a previous use of this transaction.
    m_maxWaitTicks = 0;
    m_abort = FALSE;
    m_error = SUCCESS;

    // For each sequence of transfers in the queue, or until transaction is aborted:
    pXfr = m_pFirstXfr;
    while (status && (pXfr != nullptr) && !m_abort)
    {
        // Perform the sequence of transfers.
        status = _performContiguousTransfers(pXfr);
        if (!status) { error = GetLastError(); }

        // If the next transfer has a callback routine, invoke it.
        if (status && (pXfr != nullptr) && pXfr->hasCallback())
        {
            status = pXfr->invokeCallback();
            if (!status) { error = GetLastError(); }

            if (status)
            {
                // Get the next transfer in the transaction.
                pXfr = pXfr->getNextTransfer();
            }
        }
    }

    // Signal that this transaction has been processed.
    m_isIncomplete = FALSE;

    if (!status) { SetLastError(error); }
    return status;
}

//
// Method to perform a set of transfers that happen together on the I2C bus.
// The sequence ends when a callback must be called, or at the end of the
// transaction (whichever comes first).
// INPUT:
//      pXfr - Pointer to the next transfer to process.
// OUTPUT:
//      pXfr - Pointer to a callback "transfer" or NULL if at end of transaction.
//
BOOL I2cTransactionClass::_performContiguousTransfers(I2cTransferClass* & pXfr)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* cmdXfr = nullptr;
    I2cTransferClass* readXfr = nullptr;
    PUCHAR readPtr = nullptr;
    BOOL restart = FALSE;
    ULONG cmdDat;
    UCHAR writeByte;
    UCHAR readByte;
    ULONGLONG startWaitTicks = 0;
    ULONGLONG currentTicks = 0;


    if (pXfr == nullptr)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Calculate the command and read counts for the current sequence of 
        // contigous transfers in this transaction.
        status = _calculateCurrentCounts(pXfr);
        if (!status) { error = GetLastError(); }
    }

    // For each transfer in this section of the transaction:
    cmdXfr = pXfr;
    while (status && (m_cmdsOutstanding > 0) && (cmdXfr != nullptr))
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
        while (status && (cmdXfr->getNextCmd(writeByte)))
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
                restart = FALSE;            // Only want to RESTART on first command of transfer
            }

            // If this is the last command before the end of the transaction or
            // before a callback, signal a STOP.
            if (m_cmdsOutstanding == 1)
            {
                cmdDat = cmdDat | (1 << 9);
            }
            

            g_i2c.issueCmd(cmdDat);
            m_cmdsOutstanding--;

            status = _handleErrors();
            if (!status) { error = GetLastError(); }

            // Pull any available bytes out of the receive FIFO.
            while (status && g_i2c.rxFifoNotEmtpy())
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
                else
                {
                    g_pins.setPinState(0, 1);
                    g_pins.setPinState(0, 0);
                }
            }
        }
        if (status)
        {
            cmdXfr = cmdXfr->getNextTransfer();
        }
    }

    // Complete any outstanding reads and wait for the TX FIFO to empty.
    startWaitTicks = GetTickCount64();
    currentTicks = startWaitTicks;
    while (status && ((m_readsOutstanding > 0) || !g_i2c.txFifoEmpty()) && !g_i2c.errorOccured())
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

        // Wait up to to 100 milliseconds for transfers to happen.
        if (m_readsOutstanding > 0)
        {
            currentTicks = GetTickCount64();
            if (((currentTicks - startWaitTicks) > 100) && g_i2c.rxFifoEmpty())
            {
                status = FALSE;
                error = ERROR_RECEIVE_PARTIAL;
            }
        }
    }

    // Determine if an error occured on this transaction.
    if (status)
    {
        status = _handleErrors();
        error = GetLastError();
    }

    // Pass the next transfer pointer back to the caller.
    pXfr = cmdXfr;

    // Record the read wait count for debugging purposes.
    if ((currentTicks - startWaitTicks) > m_maxWaitTicks)
    {
        m_maxWaitTicks = (ULONG) (currentTicks - startWaitTicks);
    }

    if (status)
    {
        // Check for some catch-all errors.
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
    HiResTimerClass timeOut;


    // Wait up to two milliseconds for the I2C Controller to go.  It takes less than two
    // milliseconds to transfer a full TX FIFO at "Standard" I2C bus speed (100 kbps).
    timeOut.StartTimeout(2000);
    while (g_i2c.isActive() && !timeOut.TimeIsUp());

    // Handle a bus error if we got one.
    if (m_error == SUCCESS)
    {
        status = _handleErrors();
        if (!status) { error = GetLastError(); }
    }
    if (!status) { SetLastError(error); }
    return status;
}

// Method to calculate the command and read counts for the current section
// of the transaction (up to callback or end, whichever comes first).
BOOL I2cTransactionClass::_calculateCurrentCounts(I2cTransferClass* nextXfr)
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    I2cTransferClass* pXfr = nextXfr;

    // Clear out any counts currently in place.
    m_cmdsOutstanding = 0;
    m_readsOutstanding = 0;

    // For each transfer in the queue or until a callback "transfer" is found:
    while ((pXfr != nullptr) && !pXfr->hasCallback())
    {
        // Include the size of the transfer's buffer in the command count.
        m_cmdsOutstanding = m_cmdsOutstanding + pXfr->getBufferSize();

        // If this is a read transfer, include buffer size in read count.
        if (pXfr->transferIsRead())
        {
            m_readsOutstanding = m_readsOutstanding + pXfr->getBufferSize();
        }

        // Get the next transfer.
        pXfr = pXfr->getNextTransfer();
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Determine whether an error occured, and if so (and it is the first error on this
transaction) record the error information.
\return TRUE no error occured. FALSE error occured, GetLastError() provides error code.
*/
BOOL I2cTransactionClass::_handleErrors()
{
    BOOL status = TRUE;

    // If an error occurred during this transaction:
    if (g_i2c.errorOccured())
    {
        // If error information has not yet been captured for this transaction:
        if (m_error == SUCCESS)
        {
            // Record the type of error that occured.
            if (g_i2c.addressWasNacked())
            {
                m_error = ADR_NACK;
            }
            else if (g_i2c.dataWasNacked())
            {
                m_error = DATA_NACK;
            }
            else
            {
                m_error = OTHER;
            }
        }
        // Clear the error.
        g_i2c.clearErrors();

        status = FALSE;
        SetLastError(ERROR_BUS_RESET);
    }
    return status;
}
