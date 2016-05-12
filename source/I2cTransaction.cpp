// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "I2cTransaction.h"
#include "I2cController.h"
#include "HiResTimer.h"
#include "ErrorCodes.h"
#include "DmapSupport.h"


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
    m_maxWaitTicks = 0;
    m_abort = FALSE;
    m_error = SUCCESS;
    m_isIncomplete = FALSE;
}

// Sets the 7-bit address of the slave for this tranaction.
HRESULT I2cTransactionClass::setAddress(ULONG slaveAdr)
{
    HRESULT hr = S_OK;

    if ((slaveAdr < 0x08) || (slaveAdr > 0x77))
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
        hr = DMAP_E_I2C_NO_OR_EMPTY_WRITE_BUFFER;
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
        hr = DMAP_E_I2C_NO_OR_ZERO_LENGTH_READ_BUFFER;
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
        hr = DMAP_E_I2C_NO_CALLBACK_ROUTINE_SPECIFIED;
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
HRESULT I2cTransactionClass::execute(I2cControllerClass* controller)
{
    HRESULT hr = S_OK;
    I2cTransferClass* pXfr = nullptr;
    I2cTransferClass* pReadXfr = nullptr;
    DWORD lockResult = 0;
    BOOL haveLock = FALSE;
    
    // Get the I2C Controller mapped if it is not mapped yet.
    m_controller = controller;
    hr = m_controller->mapIfNeeded();

    if (SUCCEEDED(hr))
    {
        m_hI2cLock = m_controller->getControllerHandle();
    }

    if (SUCCEEDED(hr))
    {
        // Lock the I2C bus for access exclusively by this transaction.
        hr = _acquireI2cLock();
    }

    // If we have the I2C bus locked:
    if (SUCCEEDED(hr))
    {
        // Initialize the controller.
        hr = m_controller->_initializeForTransaction(m_slaveAddress, m_useHighSpeed);

        if (SUCCEEDED(hr))
        {
            // Process each transfer on the queue.
            hr = _processTransfers();
        }

        if (SUCCEEDED(hr))
        {
            // Shut down the controller.
            hr = _shutDownI2cAfterTransaction();
        }

        // Release the I2C lock, ignoring any error returned because it is likely
        // we already have an error that we don't want to cover up.
        _releaseI2cLock();
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
        hr = m_controller->_performContiguousTransfers(pXfr);

        // Get code for any transfer error that needs to be passed back to higher code.
        m_error = m_controller->getTransfersError();

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

// Method to shut down the I2C Controller after a transaction is done with it.
HRESULT I2cTransactionClass::_shutDownI2cAfterTransaction()
{
    HRESULT hr = S_OK;
    HiResTimerClass timeOut;

    // Wait up to two milliseconds for the I2C Controller to go.  It takes less than two
    // milliseconds to transfer a full TX FIFO at "Standard" I2C bus speed (100 kbps).
    timeOut.StartTimeout(2000);
    while (m_controller->isActive() && !timeOut.TimeIsUp());

    // Handle a bus error if we got one.
    if (m_error == SUCCESS)
    {
        hr = m_controller->_handleErrors();
    }
    
    return hr;
}

/**
This lock is a global mutex when running under Win32, and a kernel mode fastmutex
(implemented in DMap.sys) when running under UWP.  This lock is used both for
cross-process and cross-thread locking.  It is only held for the duration of a
transaction.  Because of this lock (as well as the limitations of the I2C Controller)
nested I2C transactions are not allowed (for example: all needed pin MUXing must be
done before the lock is acquired to execute the I2C transaction.
\return HRESULT success or error code.
*/
HRESULT I2cTransactionClass::_acquireI2cLock()
{
    HRESULT hr = S_OK;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        hr = DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetControllerLock(m_hI2cLock);
    }
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        m_hI2cLock = CreateMutex(NULL, FALSE, L"Global\\I2c_Controller_Mutex");
        if (m_hI2cLock == NULL)
        {
            m_hI2cLock = INVALID_HANDLE_VALUE;
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
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
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    return hr;
}

/**
This routine should only be called when it is known that an I2C lock is being held
on the m_hI2cLock handle for this transaction.
\return HRESULT success or error code.
*/
HRESULT I2cTransactionClass::_releaseI2cLock()
{
    HRESULT hr = S_OK;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
    if (m_hI2cLock == INVALID_HANDLE_VALUE)
    {
        hr = DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        hr = ReleaseControllerLock(m_hI2cLock);
    }
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
    ReleaseMutex(m_hI2cLock);
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    return hr;
}
