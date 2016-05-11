// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "GpioInterrupt.h"
#include "DmapSupport.h"
#include "Robuffer.h"
//
using namespace Windows::Devices::Custom;
using namespace Windows::Storage::Streams;

/// Method to attach to an interrupt on a GPIO port bit.
HRESULT GpioInterruptsClass::attachInterrupt(ULONG pin, std::function<void(void)> func, ULONG mode, HANDLE hController)
{
    HRESULT hr = S_OK;
    static IOControlCode^ AttachIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x105, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
    HANDLE hIntController = hController;
    HANDLE hIntEnableEvent = m_hIntEnableEvent;

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        auto writer = ref new DataWriter;
        writer->ByteOrder = ByteOrder::LittleEndian;
        writer->WriteUInt16((uint16_t)pin);
        writer->WriteUInt16((uint16_t)mode);
        IBuffer^ buffer = writer->DetachBuffer();

        hr = SendIOControlCodeToController(
            hIntController,
            AttachIntCode,
            buffer,
            nullptr,
            INFINITE
            );
    }

    if (SUCCEEDED(hr))
    {
        Concurrency::create_task([pin, func, mode, hIntController, hIntEnableEvent]()
        {
            HRESULT hr = S_OK;
            static IOControlCode^ WaitIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x107, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
            auto writer = ref new DataWriter;
            writer->ByteOrder = ByteOrder::LittleEndian;
            writer->WriteUInt32(pin);
            IBuffer^ requestBuffer = writer->DetachBuffer();
            IBuffer^ replyBuffer = ref new Buffer(sizeof DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER);
            BOOL continueIo = TRUE;

            // Until the driver starts cancelling interrupt wait requests on this pin:
            do
            {
                hr = SendIOControlCodeToController(
                    hIntController,
                    WaitIntCode,
                    requestBuffer,
                    replyBuffer,
                    INFINITE
                    );

                if (hr == ERROR_OPERATION_ABORTED)
                {
                    continueIo = FALSE;
                }

                if (continueIo)
                {
                    // Wait for interrupt delivery to be enabled.
                    WaitForSingleObject(hIntEnableEvent, INFINITE);

                    // Call the interrupt callback routine.
                    func();
                }

            } while (continueIo);

            return;
        });
    }
    return hr;
}

/// Method to attach to an interrupt on a GPIO port bit with information return.
HRESULT GpioInterruptsClass::attachInterruptEx(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode, HANDLE hController)
{
    HRESULT hr = S_OK;
    static IOControlCode^ AttachIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x105, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
    HANDLE hIntController = hController;
    HANDLE hIntEnableEvent = m_hIntEnableEvent;

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        auto writer = ref new DataWriter;
        writer->ByteOrder = ByteOrder::LittleEndian;
        writer->WriteUInt16((uint16_t)pin);
        writer->WriteUInt16((uint16_t)mode);
        IBuffer^ buffer = writer->DetachBuffer();

        hr = SendIOControlCodeToController(
            hIntController,
            AttachIntCode,
            buffer,
            nullptr,
            INFINITE
            );
    }

    if (SUCCEEDED(hr))
    {
        Concurrency::create_task([pin, func, mode, hIntController, hIntEnableEvent]()
        {
            HRESULT hr = S_OK;
            static IOControlCode^ WaitIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x107, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
            auto writer = ref new DataWriter;
            writer->ByteOrder = ByteOrder::LittleEndian;
            writer->WriteUInt32(pin);
            IBuffer^ requestBuffer = writer->DetachBuffer();
            IBuffer^ replyBuffer = ref new Buffer(sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER));
            BOOL continueIo = TRUE;

            // Until the driver starts cancelling interrupt wait requests on this pin:
            do
            {
                hr = SendIOControlCodeToController(
                    hIntController,
                    WaitIntCode,
                    requestBuffer,
                    replyBuffer,
                    INFINITE
                    );

                if (hr == ERROR_OPERATION_ABORTED)
                {
                    continueIo = FALSE;
                }

                if (continueIo)
                {
                    // Make a byte array copy of the buffer sent back with the wait request completion.
                    BYTE rawBuffer[sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)];
                    auto reader = DataReader::FromBuffer(replyBuffer);
                    for (int i = 0; i < sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER); i++)
                    {
                        rawBuffer[i] = reader->ReadByte();
                    }

                    // Wait for interrupt delivery to be enabled.
                    if (WAIT_FAILED == WaitForSingleObject(hIntEnableEvent, INFINITE))
                    {
                        continueIo = FALSE;
                        hr = ERROR_OPERATION_ABORTED;
                    }
                    else
                    {
                        // Call the interrupt callback routine.
                        func((PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)rawBuffer);
                    }
                }

            } while (continueIo);

            return;
        });
    }
    return hr;
}

/// Method to attach to an interrupt on a GPIO port bit with information return and context
HRESULT GpioInterruptsClass::attachInterruptContext(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode, HANDLE hController)
{
    HRESULT hr = S_OK;
    static IOControlCode^ AttachIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x105, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
    HANDLE hIntController = hController;
    HANDLE hIntEnableEvent = m_hIntEnableEvent;

    // Tell the driver to attach the interrupt.
    if (SUCCEEDED(hr))
    {
        auto writer = ref new DataWriter;
        writer->ByteOrder = ByteOrder::LittleEndian;
        writer->WriteUInt16((uint16_t)pin);
        writer->WriteUInt16((uint16_t)mode);
        IBuffer^ buffer = writer->DetachBuffer();

        hr = SendIOControlCodeToController(
            hIntController,
            AttachIntCode,
            buffer,
            nullptr,
            INFINITE
            );
    }

    if (SUCCEEDED(hr))
    {
        Concurrency::create_task([pin, func, context, mode, hIntController, hIntEnableEvent]()
        {
            HRESULT hr = S_OK;
            static IOControlCode^ WaitIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x107, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
            auto writer = ref new DataWriter;
            writer->ByteOrder = ByteOrder::LittleEndian;
            writer->WriteUInt32(pin);
            IBuffer^ requestBuffer = writer->DetachBuffer();
            IBuffer^ replyBuffer = ref new Buffer(sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER));
            BOOL continueIo = TRUE;

            // Until the driver starts cancelling interrupt wait requests on this pin:
            do
            {
                hr = SendIOControlCodeToController(
                    hIntController,
                    WaitIntCode,
                    requestBuffer,
                    replyBuffer,
                    INFINITE
                    );

                if (hr == ERROR_OPERATION_ABORTED)
                {
                    continueIo = FALSE;
                }

                if (continueIo)
                {
                    // Make a byte array copy of the buffer sent back with the wait request completion.
                    BYTE rawBuffer[sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)];
                    auto reader = DataReader::FromBuffer(replyBuffer);
                    for (int i = 0; i < sizeof(DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER); i++)
                    {
                        rawBuffer[i] = reader->ReadByte();
                    }

                    // Wait for interrupt delivery to be enabled.
                    if (WAIT_FAILED == WaitForSingleObject(hIntEnableEvent, INFINITE))
                    {
                        continueIo = FALSE;
                        hr = ERROR_OPERATION_ABORTED;
                    }
                    else
                    {
                        // Call the interrupt callback routine.
                        func((PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)rawBuffer, context);
                    }
                }

            } while (continueIo);

            return;
        });
    }
    return hr;
}
/// Method to detach an interrupt for a GPIO port bit.
HRESULT GpioInterruptsClass::detachInterrupt(ULONG pin, HANDLE hController)
{
    HRESULT hr = S_OK;
    static IOControlCode^ DetachIntCode = ref new IOControlCode(FILE_DEVICE_DMAP, 0x106, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
    HANDLE hIntController = hController;

    // Tell the driver to detach the interrupt.
    if (SUCCEEDED(hr))
    {
        auto writer = ref new DataWriter;
        writer->ByteOrder = ByteOrder::LittleEndian;
        writer->WriteUInt32(pin);
        IBuffer^ buffer = writer->DetachBuffer();

        hr = SendIOControlCodeToController(
            hIntController,
            DetachIntCode,
            buffer,
            nullptr,
            INFINITE
            );
    }

    return hr;
}
