// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "DmapSupport.h"

/**
Get the base address of a memory mapped controller in the SOC.  Exclusive, non-shared 
access to the controller is requested.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[out] baseAddress Base address of the controller in question.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress)
{
    return GetControllerBaseAddress(deviceName, handle, baseAddress, 0);
}

/**
Get the base address of a memory mapped controller in the SOC.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[out] baseAddress Base address of the controller in question.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    DMAP_MAPMEMORY_OUTPUT_BUFFER buf = { 0 };
    DWORD bytesReturned = 0;

    status = OpenControllerDevice(deviceName, handle, shareMode);
    if (!status) { error = GetLastError(); }

    if (status && (baseAddress == nullptr))
    {
        // Retrieve the base address of controller registers.
        status = DeviceIoControl(
            handle,
            IOCTL_DMAP_MAPMEMORY,
            nullptr,
            0,
            &buf,
            sizeof(buf),
            &bytesReturned,
            nullptr);
        if (!status)
        {
            error = GetLastError();
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
            baseAddress = nullptr;
        }
        else
        {
            // Pass back the base address of the controller registers to the caller.
            baseAddress = buf.Address;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Open a controller device in the SOC.  This is used for both memory mapped and
IO mapped controllers.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL OpenControllerDevice(PWCHAR deviceName, HANDLE & handle, DWORD shareMode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (handle == INVALID_HANDLE_VALUE)
    {
        // Open the specified device.
        handle = CreateFile(
            deviceName,
            GENERIC_READ | GENERIC_WRITE,
            shareMode,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE)
        {
            status = FALSE;
            error = GetLastError();
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}
