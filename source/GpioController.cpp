// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.


#include "pch.h"

#include "ErrorCodes.h"
#include "GpioController.h"
#include "DmapSupport.h"

using namespace Windows::Devices::Custom;
using namespace Windows::Storage::Streams;

#if defined(_M_IX86) || defined(_M_X64)

// 
// Global extern exports
//

/// The global object used to interact with the BayTrail Fabric GPIO hardware.
BtFabricGpioControllerClass g_btFabricGpio;

/**
\return HRESULT success or error code.
*/
HRESULT BtFabricGpioControllerClass::_mapS0Controller()
{
    HRESULT hr = S_OK;
    PVOID baseAddress = nullptr;

    hr = GetControllerBaseAddress(
        mbmGpioS0DeviceName,
        m_hS0Controller,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);

    if (SUCCEEDED(hr))
    {
        m_s0Controller = (PGPIO_PAD)baseAddress;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
\return HRESULT success or error code.
*/
HRESULT BtFabricGpioControllerClass::_mapS5Controller()
{
    HRESULT hr = S_OK;
    
    PVOID baseAddress = nullptr;

    hr = GetControllerBaseAddress(
        mbmGpioS5DeviceName,
        m_hS5Controller,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);

    if (SUCCEEDED(hr))
    {
        m_s5Controller = (PGPIO_PAD)baseAddress;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_ARM)
// 
// Global extern exports
//
BcmGpioControllerClass g_bcmGpio;

/**
\return HRESULT success or error code.
*/
HRESULT BcmGpioControllerClass::_mapController()
{
    HRESULT hr = S_OK;
    PVOID baseAddress = nullptr;

    hr = GetControllerBaseAddress(
        pi2GpioDeviceName,
        m_hController,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);

    if (SUCCEEDED(hr))
    {
        m_registers = (PBCM_GPIO)baseAddress;
    }

    return hr;
}

HRESULT BcmGpioControllerClass::mapIfNeeded()
{
    HRESULT hr = S_OK;

    if (m_hController == INVALID_HANDLE_VALUE)
    {
        hr = _mapController();
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S0 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS0Interrupt(ULONG intNo, std::function<void(void)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS0IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterrupt(intNo, func, mode, m_hS0Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S0 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS0InterruptEx(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS0IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptEx(intNo, func, mode, m_hS0Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S0 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS0InterruptContext(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS0IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptContext(intNo, func, context, mode, m_hS0Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S5 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS5Interrupt(ULONG intNo, std::function<void(void)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS5IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterrupt(intNo, func, mode, m_hS5Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S5 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS5InterruptEx(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS5IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptEx(intNo, func, mode, m_hS5Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to attach to an interrupt on an S5 GPIO port bit.
HRESULT BtFabricGpioControllerClass::attachS5InterruptContext(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapS5IfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptContext(intNo, func, context, mode, m_hS5Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_ARM)
/// Method to attach to an interrupt on a GPIO port bit.
HRESULT BcmGpioControllerClass::attachInterrupt(ULONG intNo, std::function<void(void)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapIfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterrupt(intNo, func, mode, m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/// Method to attach to an interrupt on a GPIO port bit with informaton return.
HRESULT BcmGpioControllerClass::attachInterruptEx(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapIfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptEx(intNo, func, mode, m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)


#if defined(_M_ARM)
/// Method to attach to an interrupt on a GPIO port bit with informaton return and context.
HRESULT BcmGpioControllerClass::attachInterruptContext(ULONG intNo, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode)
{
    HRESULT hr = S_OK;

    hr = mapIfNeeded();

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.attachInterruptContext(intNo, func, context, mode, m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/// Method to detach an interrupt for a GPIO port bit.
HRESULT BcmGpioControllerClass::detachInterrupt(ULONG intNo)
{
    HRESULT hr = S_OK;

    hr = mapIfNeeded();

    // Tell the driver to detach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.detachInterrupt(intNo, m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to detach an interrupt for an S0 GPIO port bit.
HRESULT BtFabricGpioControllerClass::detachS0Interrupt(ULONG intNo)
{
    HRESULT hr = S_OK;

    hr = mapS0IfNeeded();

    // Tell the driver to detach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.detachInterrupt(intNo, m_hS0Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/// Method to detach an interrupt for an S5 GPIO port bit.
HRESULT BtFabricGpioControllerClass::detachS5Interrupt(ULONG intNo)
{
    HRESULT hr = S_OK;

    hr = mapS5IfNeeded();

    // Tell the driver to detach the interrupt.
    if (SUCCEEDED(hr))
    {
        hr = m_gpioInterrupts.detachInterrupt(intNo, m_hS5Controller);
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)
