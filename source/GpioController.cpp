// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "GpioController.h"

/**
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL QuarkFabricGpioControllerClass::_mapController()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    PVOID baseAddress = nullptr;

    status = GetControllerBaseAddress(
        galileoGpioDeviceName,
        m_hController,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }
    else
    {
        m_controller = (PFABRIC_GPIO)baseAddress;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL BtFabricGpioControllerClass::_mapS0Controller()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    PVOID baseAddress = nullptr;

    status = GetControllerBaseAddress(
        mbmGpioS0DeviceName,
        m_hS0Controller,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }
    else
    {
        m_s0Controller = (PGPIO_PAD)baseAddress;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL BtFabricGpioControllerClass::_mapS5Controller()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    PVOID baseAddress = nullptr;

    status = GetControllerBaseAddress(
        mbmGpioS5DeviceName,
        m_hS5Controller,
        baseAddress,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }
    else
    {
        m_s5Controller = (PGPIO_PAD)baseAddress;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL QuarkLegacyGpioControllerClass::_openController()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;

    status = OpenControllerDevice(
        galileoLegacyGpioDeviceName,
        m_hController,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }

    if (!status) { SetLastError(error); }
    return status;
}
