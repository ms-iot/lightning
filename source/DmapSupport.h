// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once

#include "DMap.h"

// Define the device name strings used to access the controllers on the MBM.
#define mbmGpioS0DeviceName   L"\\\\.\\ACPI#INT33FC#1#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define mbmGpioS5DeviceName   L"\\\\.\\ACPI#INT33FC#3#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define mbmPwm0DeviceName     L"\\\\.\\ACPI#80860F09#1#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define mbmPwm1DeviceNmae     L"\\\\.\\ACPI#80860F09#2#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define mbmSpiDeviceName      L"\\\\.\\ACPI#80860F0E#0#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define mbmI2cDeviceName      L"\\\\.\\ACPI#80860F41#6#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"

// Define the device name strings used to access the controllers on the PI2.
#define pi2Spi0DeviceName     L"\\\\.\\ACPI#BCM2838#0#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define pi2Spi1DeviceName     L"\\\\.\\ACPI#BCM2839#1#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define pi2I2c0DeviceName     L"\\\\.\\ACPI#BCM2841#0#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define pi2I2c1DeviceName     L"\\\\.\\ACPI#BCM2841#1#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define pi2PwmDeviceName      L"\\\\.\\ACPI#BCM2844#0#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define pi2GpioDeviceName     L"\\\\.\\ACPI#BCM2845#0#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"

/// Routine to get the base address of a memory mapped controller with no sharing allowed.
HRESULT GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress);

/// Routine to get the base address of a memory mapped controller with a sharing specification.
HRESULT GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode);

/// Routine to close a controller that has previously been opened.
LIGHTNING_DLL_API void DmapCloseController(HANDLE & handle);

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
/// Routine to open a controller device in the SOC.
HRESULT OpenControllerDevice(PWCHAR deviceName, HANDLE & handle, DWORD shareMode);
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app.
HRESULT SendIOControlCodeToController(
    HANDLE handle,
    Windows::Devices::Custom::IOControlCode^ iOControlCode,
    Windows::Storage::Streams::IBuffer^ bufferToDriver,
    Windows::Storage::Streams::IBuffer^ bufferFromDriver,
    uint32_t timeOutMillis);
HRESULT GetControllerLock(HANDLE & handle);
HRESULT ReleaseControllerLock(HANDLE & handle);
#endif
