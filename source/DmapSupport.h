// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _DMAP_SUPPORT_H_
#define _DMAP_SUPPORT_H_

#include <Windows.h>

#include "public.h"     // TODO: include this file in the package

#define dmapSpi0DeviceName L"\\\\.\\PCI#VEN_8086&DEV_0935&SUBSYS_09358086&REV_10#3&b1bfb68&0&A8#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapSpi1DeviceName L"\\\\.\\PCI#VEN_8086&DEV_0935&SUBSYS_09358086&REV_10#3&b1bfb68&0&A9#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapI2cDeviceName L"\\\\.\\PCI#VEN_8086&DEV_0934&SUBSYS_09348086&REV_10#3&b1bfb68&0&AA#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\0"
#define dmapGpioDeviceName L"\\\\.\\PCI#VEN_8086&DEV_0934&SUBSYS_09348086&REV_10#3&b1bfb68&0&AA#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\\1"

//
// Routine to get the base address of a controller in the SOC.
// Exclusive, non-shared, access to the controller is requested.
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
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress);

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
BOOL GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode);

#endif // _DMAP_SUPPORT_H_