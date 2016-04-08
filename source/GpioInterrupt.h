// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _GPIO_INTERRUPT_H_
#define _GPIO_INTERRUPT_H_

#include <Windows.h>
#include <functional>

#include "DMap.h"

/// Class used to control and receive GPIO interrupts.
class GpioInterruptsClass
{
public:
    /// Constructor.
    GpioInterruptsClass()
    {
        m_hIntEnableEvent = CreateEvent(NULL, TRUE, TRUE, L"Local\\DMapIntEvent");
        if (m_hIntEnableEvent == NULL)
        {
            m_hIntEnableEvent = INVALID_HANDLE_VALUE;
        }
    }

    /// Destructor.
    virtual ~GpioInterruptsClass()
    {
        if ((m_hIntEnableEvent != INVALID_HANDLE_VALUE) && (m_hIntEnableEvent != NULL))
        {
            CloseHandle(m_hIntEnableEvent);
            m_hIntEnableEvent = INVALID_HANDLE_VALUE;
        }
    }

    /// Method to attach to an interrupt on a GPIO port bit.
    HRESULT attachInterrupt(ULONG pin, std::function<void(void)> func, ULONG mode, HANDLE hController);

    /// Method to attach to an interrupt on a GPIO port bit with information return.
    HRESULT attachInterruptEx(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode, HANDLE hController);

    /// Method to attach to an interrupt on a GPIO port bit with information return.
    HRESULT attachInterruptContext(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode, HANDLE hController);

    /// Method to detach an interrupt for a GPIO port bit.
    HRESULT detachInterrupt(ULONG pin, HANDLE hController);

    /// Method to enable delivery of GPIO interrupts.
    inline HRESULT enableInterrupts()
    {
        HRESULT hr = S_OK;
        if (SetEvent(m_hIntEnableEvent) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        return hr;
    }

    /// Method to disable delivery of GPIO interrupts.
    inline HRESULT disableInterrupts()
    {
        HRESULT hr = S_OK;
        if (ResetEvent(m_hIntEnableEvent) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        return hr;
    }

private:

    /// Handle to the event used to enable and disable interrupt delivery.
    /**
    The event represented by this handle is set to the signaled state to enable interrupts.
    */
    HANDLE m_hIntEnableEvent;

    //
    // GpioInterruptsClass private methods.
    //
};

#endif  // _GPIO_INTERRUPT_H_
