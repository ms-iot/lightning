// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _DMAP_SUPPORT_H_
#define _DMAP_SUPPORT_H_

#include <Windows.h>

#include "public.h"     // TODO: include this file in the package

// Define the device name strings used to access the SPI, I2C and Fabric GPIO controllers with DMap.
#define dmapSpi0DeviceName L"\\\\.\\PCI#VEN_8086&DEV_0935&SUBSYS_09358086&REV_10#3&b1bfb68&0&A8#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapSpi1DeviceName L"\\\\.\\PCI#VEN_8086&DEV_0935&SUBSYS_09358086&REV_10#3&b1bfb68&0&A9#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapI2cDeviceName L"\\\\.\\PCI#VEN_8086&DEV_0934&SUBSYS_09348086&REV_10#3&b1bfb68&0&AA#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapGpioDeviceName L"\\\\.\\PCI#VEN_8086&DEV_0934&SUBSYS_09348086&REV_10#3&b1bfb68&0&AA#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\1"

// Define the device name string used to access the Legacy GPIO controller with DMap.
#define legacyGpioDeviceName L"\\\\.\\ACPI#INT3488#4&431f7f5&0#{091a7d51-bb55-42e4-ae25-1d0b563fa177}"

/// Routine to get the base address of a memory mapped controller with no sharing allowed.
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress);

/// Routine to get the base address of a memory mapped controller with a sharing specification.
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode);

/// Routine to open a controller device in the SOC.
BOOL OpenControllerDevice(PWCHAR deviceName, HANDLE & handle, DWORD shareMode);

#endif // _DMAP_SUPPORT_H_