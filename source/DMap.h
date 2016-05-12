/*++

Module Name:

DMap.h

Abstract:

This module contains the common declarations shared by driver
and user applications.

Environment:

user and kernel

--*/
#ifndef _DMAP_H_
#define _DMAP_H_

#include <stdint.h>
#include "Lightning.h"

#define DMAP_NAME L"DmapGpio"

#define DMAP_SYMBOLIC_NAME L"\\DosDevices\\" DMAP_NAME
#define DMAP_USERMODE_PATH L"\\\\.\\" DMAP_NAME
#define DMAP_USERMODE_PATH_SIZE sizeof(DMAP_USERMODE_PATH)


//
// Define an Interface Guid so that app can find the device and talk to it.
// {109b86ad-f53d-4b76-aa5f-821e2ddf2141}
//
#define DMAP_INTERFACE {0x109b86ad,0xf53d,0x4b76,0xaa,0x5f,0x82,0x1e,0x2d,0xdf,0x21,0x41}
DEFINE_GUID(GUID_DEVINTERFACE_DMap, 0x109b86ad, 0xf53d, 0x4b76, 0xaa, 0x5f, 0x82, 0x1e, 0x2d, 0xdf, 0x21, 0x41);

#define FILE_DEVICE_DMAP 0x423

#define IOCTL_DMAP_MAPMEMORY              CTL_CODE(FILE_DEVICE_DMAP, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMAP_WRITEPORT              CTL_CODE(FILE_DEVICE_DMAP, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMAP_READPORT               CTL_CODE(FILE_DEVICE_DMAP, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMAP_LOCK                   CTL_CODE(FILE_DEVICE_DMAP, 0x103, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DMAP_UNLOCK                 CTL_CODE(FILE_DEVICE_DMAP, 0x104, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DMAP_ATTACH_INTERRUPT       CTL_CODE(FILE_DEVICE_DMAP, 0x105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMAP_DETACH_INTERRUPT       CTL_CODE(FILE_DEVICE_DMAP, 0x106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DMAP_WAIT_INTERRUPT         CTL_CODE(FILE_DEVICE_DMAP, 0x107, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _DMAP_MAPMEMORY_OUTPUT_BUFFER
{
    void*    Address;
    uint32_t Length;
} DMAP_MAPMEMORY_OUTPUT_BUFFER, *PDMAP_MAPMEMORY_OUTPUT_BUFFER;

// Note: These must be bit fields, the first two of which combine to create the third.
#define DMAP_INTERRUPT_MODE_RISING  0x1
#define DMAP_INTERRUPT_MODE_FALLING 0x2
#define DMAP_INTERRUPT_MODE_EITHER  0x3

// Struct for the contents of a buffer sent to the driver with IOCTL_DMAP_ATTACH_INTERRUPT
// to request that future state transitions of the given pin generate interrupts.
typedef struct _DMAP_ATTACH_INTERRUPT_BUFFER
{
    uint16_t IntNo;         // Interrupt number for which to generate interrupts
    uint16_t Mode;          // Type of pin events that should cause interrupts
} DMAP_ATTACH_INTERRUPT_BUFFER, *PDMAP_ATTACH_INTERRUPT_BUFFER;

// Struct for the contents of a buffer sent to the driver with IOCTL_DMAP_DETACH_INTERRUPT
// to request that future state transistions of the given pin no longer generate interrupts.
typedef struct _DMAP_DETACH_INTERRUPT_BUFFER
{
    uint32_t IntNo;         // Interrupt number which interrupts are no longer wanted
} DMAP_DETACH_INTERRUPT_BUFFER, *PDMAP_DETACH_INTERRUPT_BUFFER;

// Struct for the contents of a buffer sent to the driver with IOCTL_DMAP_WAIT_INTERRUPT
// to request notification for the next (currently queued, or future) interrupt from the 
// specified pin.
typedef struct _DMAP_WAIT_INTERRUPT_REQUEST_BUFFER
{
    uint32_t IntNo;         // Which interrupt to wait for
} DMAP_WAIT_INTERRUPT_REQUEST_BUFFER, *PDMAP_WAIT_INTERRUPT_REQUEST_BUFFER;

// Struct for the contents of a buffer sent by the driver on completion of an I/O operation 
// with IOCTL_DMAP_WAIT_INTERRUPT to notify the waiting thread that an interrupt has occurred.
typedef struct _DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER
{
    uint16_t IntNo;         // The number of the source for this interrupt
    uint16_t NewState;      // The new state of the pin after the interrupt event
    uint32_t DropCount;     // The count of interrupts dropped since the last interrupt notification
    uint64_t EventTime;     // High resolution timer captured shortly after the pin event
} DMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, *PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER;

#endif // #ifndef _DMAP_H_
