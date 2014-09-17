// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "GpioController.h"


/**
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL FabricGpioControllerClass::_mapController()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;
    PVOID baseAddress = nullptr;

    status = GetControllerBaseAddress(
        dmapGpioDeviceName,
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
BOOL LegacyGpioControllerClass::_openController()
{
    BOOL status = TRUE;
    BOOL error = ERROR_SUCCESS;

    status = OpenControllerDevice(
        legacyGpioDeviceName,
        m_hController,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (!status)
    {
        error = GetLastError();
    }

    if (!status) { SetLastError(error); }
    return status;
}
