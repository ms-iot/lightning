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
HRESULT I2cControllerClass::beginExternal()
{
    HRESULT hr = S_OK;
 
    // Set the MUXes for external I2C use.
    hr = g_pins.verifyPinFunction(ARDUINO_PIN_I2C_CLK, FUNC_I2C, BoardPinsClass::LOCK_FUNCTION);

    if (SUCCEEDED(hr))
    {
        hr = g_pins.verifyPinFunction(ARDUINO_PIN_I2C_DAT, FUNC_I2C, BoardPinsClass::LOCK_FUNCTION);
    }

    return hr;
}

// This method returns the esternal I2C bus pins to their default configurations.
void I2cControllerClass::endExternal()
{
    // Set the pns used for I2C back to Digital inputs, on a best effort basis.
    g_pins.verifyPinFunction(ARDUINO_PIN_I2C_DAT, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
    g_pins.setPinMode(ARDUINO_PIN_I2C_DAT, DIRECTION_IN, false);
    g_pins.verifyPinFunction(ARDUINO_PIN_I2C_CLK, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
    g_pins.setPinMode(ARDUINO_PIN_I2C_CLK, DIRECTION_IN, false);
}

// This method maps the I2C controller if needed.
inline HRESULT I2cControllerClass::mapIfNeeded()
{
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        return _mapController();
    }
    else
    {
        return S_OK;
    }
}

// Method to map the I2C controller into this process' virtual address space.
HRESULT I2cControllerClass::_mapController()
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
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN1:
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN2:
            deviceName = galileoI2cDeviceName;
            break;
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
            m_controller = (PI2C_CONTROLLER)baseAddress;
        }
    }
    
    return hr;
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
HRESULT I2cTransactionClass::setAddress(ULONG slaveAdr)
{
    HRESULT hr = S_OK;

    // Verify we successfully create the I2C Controller Lock.
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
		hr = DMAP_E_I2C_LOCK_CREATION_FAILED;
    }

    if (SUCCEEDED(hr) && ((slaveAdr < 0x08) || (slaveAdr >= 0x77)))
    {
        hr = DMAP_E_I2C_ADDRESS_OUT_OF_RANGE;
    }

    if (SUCCEEDED(hr))
    {
        m_slaveAddress = slaveAdr;
    }
    
    return hr;
}

// Add a write transfer to the transaction.
HRESULT I2cTransactionClass::queueWrite(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;

    // Sanity check the buffer and size parameters.
    if ((buffer == nullptr) || (bufferBytes == 0))
    {
        hr = DMAP_E_NO_OR_EMPTY_WRITE_BUFFER;
    }

    if (SUCCEEDED(hr))
    {
        // Allocate a transfer object.
        pXfr = new I2cTransferClass;

        if (pXfr == 0)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
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
    
    return hr;
}

// Add a read transfer to the transaction.
HRESULT I2cTransactionClass::queueRead(PUCHAR buffer, ULONG bufferBytes, BOOL preRestart)
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;

    // Sanity check the buffer and size parameters.
    if ((buffer == nullptr) || (bufferBytes == 0))
    {
        hr = DMAP_E_NO_OR_ZERO_LENGTH_READ_BUFFER;
    }

    if (SUCCEEDED(hr))
    {
        // Allocate a transfer object.
        pXfr = new I2cTransferClass;

        if (pXfr == 0)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
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
    
    return hr;
}

// Method to queue a callback routine at the current point in the transaction.
HRESULT I2cTransactionClass::queueCallback(std::function<HRESULT()> callBack)
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;

    if (callBack == nullptr)
    {
        hr = DMAP_E_NO_CALLBACK_ROUTINE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        // Allocate a transfer object.
        pXfr = new I2cTransferClass;

        if (pXfr == 0)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Associate the callback with the transfer.
        hr = pXfr->setCallback(callBack);
    }

    if (SUCCEEDED(hr))
    {
        // Queue the transfer as part of this transaction.
        _queueTransfer(pXfr);

        // Indicate this transaction has at least one incomplete "transfer."
        m_isIncomplete = TRUE;
    }
    
    return hr;
}

// Method to perform the transfers associated with this transaction.
HRESULT I2cTransactionClass::execute()
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;
    I2cTransferClass* pReadXfr = nullptr;
    DWORD lockResult = 0;
    BOOL haveLock = FALSE;

    // Verify we successfully created the I2C Controller Lock.
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        hr = DMAP_E_I2C_LOCK_CREATION_FAILED;
    }

    if (SUCCEEDED(hr))
    {
        // Claim the I2C controller.
        lockResult = WaitForSingleObject(m_hI2cLock, 5000);
        if ((lockResult == WAIT_OBJECT_0) || (lockResult == WAIT_ABANDONED))
        {
            haveLock = TRUE;
        }
        else if (lockResult == WAIT_TIMEOUT)
        {
            hr = DMAP_E_I2C_BUS_LOCK_TIMEOUT;
        }
        else
        {
			hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Initialize the controller.
        hr = _initializeI2cForTransaction();
    }

    // Process each transfer on the queue.
    if (SUCCEEDED(hr))
    {
        hr = _processTransfers();
    }

    if (SUCCEEDED(hr))
    {
        // Shut down the controller.
        hr = _shutDownI2cAfterTransaction();
    }

    // Release this transaction's claim on the controller.
    if (haveLock)
    {
        ReleaseMutex(m_hI2cLock);
        haveLock = FALSE;
    }
    
    return hr;
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
HRESULT I2cTransactionClass::_initializeI2cForTransaction()
{
    HRESULT hr = S_OK;
    ULONGLONG waitStartTicks = 0;

    // Get the I2C Controller mapped if it is not mapped yet.
    hr = g_i2c.mapIfNeeded();

    if (SUCCEEDED(hr) && (!g_i2c.isInitialized() || (g_i2c.getAddress() != m_slaveAddress)))
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
    
    return hr;
}

// Method to process the transfers in this transaction.
HRESULT I2cTransactionClass::_processTransfers()
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;

    // Clear out any data from a previous use of this transaction.
    m_maxWaitTicks = 0;
    m_abort = FALSE;
    m_error = SUCCESS;

    // For each sequence of transfers in the queue, or until transaction is aborted:
    pXfr = m_pFirstXfr;
    while (SUCCEEDED(hr) && (pXfr != nullptr) && !m_abort)
    {
        // Perform the sequence of transfers.
        hr = _performContiguousTransfers(pXfr);

        // If the next transfer has a callback routine, invoke it.
        if (SUCCEEDED(hr) && (pXfr != nullptr) && pXfr->hasCallback())
        {
            hr = pXfr->invokeCallback();

            if (SUCCEEDED(hr))
            {
                // Get the next transfer in the transaction.
                pXfr = pXfr->getNextTransfer();
            }
        }
    }

    // Signal that this transaction has been processed.
    m_isIncomplete = FALSE;
    
    return hr;
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
HRESULT I2cTransactionClass::_performContiguousTransfers(I2cTransferClass* & pXfr)
{
    HRESULT hr = S_OK;
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
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the command and read counts for the current sequence of 
        // contigous transfers in this transaction.
        hr = _calculateCurrentCounts(pXfr);
    }

    // For each transfer in this section of the transaction:
    cmdXfr = pXfr;
    while (SUCCEEDED(hr) && (m_cmdsOutstanding > 0) && (cmdXfr != nullptr))
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
		while (SUCCEEDED(hr) && (cmdXfr->getNextCmd(writeByte)))
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

            hr = _handleErrors();

            // Pull any available bytes out of the receive FIFO.
			while (SUCCEEDED(hr) && g_i2c.rxFifoNotEmtpy())
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

        if (SUCCEEDED(hr))
        {
            cmdXfr = cmdXfr->getNextTransfer();
        }
    }

    // Complete any outstanding reads and wait for the TX FIFO to empty.
    startWaitTicks = GetTickCount64();
    currentTicks = startWaitTicks;
	while (SUCCEEDED(hr) && ((m_readsOutstanding > 0) || !g_i2c.txFifoEmpty()) && !g_i2c.errorOccured())
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
        m_maxWaitTicks = (ULONG) (currentTicks - startWaitTicks);
    }

    if (SUCCEEDED(hr))
    {
        // Check for some catch-all errors.
        if (m_cmdsOutstanding > 0)
        {
            hr = DMAP_E_I2C_OPERATION_INCOMPLETE;
        }
        else if (m_readsOutstanding < 0)
        {
            hr = DMAP_E_I2C_EXTRA_DATA_RECEIVED;
        }
        else if (m_cmdsOutstanding < 0)
        {
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }
    
    return hr;
}

// Method to shut down the I2C Controller after a transaction is done with it.
HRESULT I2cTransactionClass::_shutDownI2cAfterTransaction()
{
    HRESULT hr = S_OK;
    HiResTimerClass timeOut;

    // Wait up to two milliseconds for the I2C Controller to go.  It takes less than two
    // milliseconds to transfer a full TX FIFO at "Standard" I2C bus speed (100 kbps).
    timeOut.StartTimeout(2000);
    while (g_i2c.isActive() && !timeOut.TimeIsUp());

    // Handle a bus error if we got one.
    if (m_error == SUCCESS)
    {
        hr = _handleErrors();
    }
    
    return hr;
}

// Method to calculate the command and read counts for the current section
// of the transaction (up to callback or end, whichever comes first).
HRESULT I2cTransactionClass::_calculateCurrentCounts(I2cTransferClass* nextXfr)
{
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
    
    return S_OK;
}

/**
Determine whether an error occured, and if so (and it is the first error on this
transaction) record the error information.
\return HRESULT success or error code.
*/
HRESULT I2cTransactionClass::_handleErrors()
{
    HRESULT hr = S_OK;

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

        hr = E_FAIL;
        SetLastError(ERROR_BUS_RESET);
    }
    return hr;
}
