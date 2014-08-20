// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "DmapSupport.h"

//
// Routine to get the base address of a controller in the SOC.
//
// INPUT:
//  deviceName - The name of the PCI device used to map the controller in question.
//
// OUTPUT:
//  handle - Handle opened to the device specified by deviceName.
//  baseAddress - Base address of the controller in questions.
//
// RETURN:
//  TRUE - Success
//  FALSE - An error occurred, use GetLastError() to get more information.
//
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress)
{
    return GetControllerBaseAddress(deviceName, handle, baseAddress, 0);
}

//
// Routine to get the base address of a controller in the SOC with a sharing specification.
//
// INPUT:
//  deviceName - The name of the PCI device used to map the controller in question.
//
// OUTPUT:
//  handle - Handle opened to the device specified by deviceName.
//  baseAddress - Base address of the controller in questions.
//  shareMode - Sharing specifier as specified to Createfile().
//
// RETURN:
//  TRUE - Success
//  FALSE - An error occurred, use GetLastError() to get more information.
//
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    DMAP_MAPMEMORY_OUTPUT_BUFFER buf = { 0 };
    DWORD bytesReturned = 0;

    if (handle == INVALID_HANDLE_VALUE)
    {
        // Open the Dmap device for the controller.
        handle = CreateFile(
            deviceName,
            GENERIC_READ | GENERIC_WRITE,
            shareMode,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE)
        {
            status = FALSE;
            error = GetLastError();
        }
    }

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

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

