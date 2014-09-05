// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _GPIO_CONTROLLER_H_
#define _GPIO_CONTROLLER_H_

#include <Windows.h>
#include "ArduinoCommon.h"
#include "DmapSupport.h"

//
// Class that is used to interact with the Fabric GPIO hardware.
//
class FabricGpioControllerClass
{
public:
    FabricGpioControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
        m_controller = nullptr;
    }

    virtual ~FabricGpioControllerClass()
    {
        if (m_hController != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hController);
            m_hController = INVALID_HANDLE_VALUE;
        }
    }

    // Method to map the controller registers if they are not already mapped.
    inline BOOL mapIfNeeded()
    {
        if (m_hController == INVALID_HANDLE_VALUE)
        {
            return _mapController();
        }
        else
        {
            return TRUE;
        }
    }

    // Method to set the state of a Fabric GPIO pin.
    // This method assumes portBit is in the valid range of 0-7, and that
    // state is either 0 or 1.
    inline BOOL setPinState(ULONG portBit, ULONG state)
    {
        BOOL status = TRUE;
        DWORD error = ERROR_SUCCESS;

        status = mapIfNeeded();

        if (status)
        {
            if (state == 0)
            {
                _interlockedbittestandreset((LONG*)&m_controller->GPIO_SWPORTA_DR.ALL_BITS, portBit);
            }
            else
            {
                _interlockedbittestandset((LONG*)&m_controller->GPIO_SWPORTA_DR.ALL_BITS, portBit);
            }
        }

        return status;
    }

    // Method to read the state of a Fabric GPIO pin.
    // This method assumes portBit is in the valid range of 0-7.
    inline BOOL getPinState(ULONG portBit, ULONG & state)
    {
        BOOL status = TRUE;
        DWORD error = ERROR_SUCCESS;

        status = mapIfNeeded();

        if (status)
        {
            state = (m_controller->GPIO_EXT_PORTA.ALL_BITS >> portBit) & 0x01;
        }

        return status;
    }

    // Method to set the direction (input or output) of a Fabric GPIO pin.
    // This routine assumes mode is INPUT or OUTPUT.
    inline BOOL setPinDirection(ULONG portBit, ULONG mode)
    {
        BOOL status = TRUE;
        DWORD error = ERROR_SUCCESS;

        if (portBit > 7)
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        if (status)
        {
            status = mapIfNeeded();
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            switch (mode)
            {
            case DIRECTION_IN:
                _setPinInput(portBit);
                break;
            case DIRECTION_OUT:
                _setPinOutput(portBit);
                break;
            }
        }

        if (!status) { SetLastError(error); }
        return status;
    }

private:

    #pragma warning(push)
    #pragma warning(disable : 4201) // Ignore nameless struct/union warnings

    // Port A Data Register.
    typedef union {
        struct {
            ULONG GPIO_SWPORTA_DR : 8;  // Port Data
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_SWPORTA_DR;

    // Port A Data Direction Register.
    typedef union {
        struct {
            ULONG GPIO_SWPORTA_DDR : 8;     // Port Data Direction
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_SWPORTA_DDR;

    // Interrupt Enable Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTEN : 8;           // Interrupt Enable
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTEN;

    // Interrupt Mask Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTMASK : 8;         // Interrupt Mask
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTMASK;

    // Interrupt Type Register for Port A.
    typedef union {
        struct {
            ULONG GPI_INTTYPE_LEVEL : 8;    // Interrupt Type
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTTYPE_LEVEL;

    // Interrupt Polarity Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INT_POLARITY : 8;    // Interrupt Polarity
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INT_POLARITY;

    // Interrupt Status Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTSTATUS : 8;       // Interrupt Status
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTSTATUS;

    // Raw Interrupt Status Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_RAW_INSTATUS : 8;    // Raw Interrupt Status
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_RAW_INTSTATUS;

    // Debounce Enable Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_DEBOUNCE : 8;        // Debounce Enable
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_DEBOUNCE;

    // Clear Interrupt Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_PORTA_EOI : 8;       // Clear Interrupt
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_PORTA_EOI;

    // Port A External Port Register.
    typedef union {
        struct {
            ULONG GPIO_EXT_PORTA : 8;       // External Port
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_EXT_PORTA;

    // Register controls Synchronization of Level-Sensitive interrupts.
    typedef union {
        struct {
            ULONG GPIO_LS_SYNC : 1;         // Synchronization Level
            ULONG _reserved : 31;
        };
        ULONG ALL_BITS;
    } _GPIO_LS_SYNC;

    #pragma warning( pop )

    // Layout of the Quark Fabric GPIO Controller registers in memory.
    typedef struct _FABRIC_GPIO {
        _GPIO_SWPORTA_DR        GPIO_SWPORTA_DR;    // 0x00 - Port A Data
        _GPIO_SWPORTA_DDR       GPIO_SWPORTA_DDR;   // 0x04 - Port A Data Direction
        ULONG                   _reserved1[0x28];   // 0x08 - 0x2F
        _GPIO_INTEN             GPIO_INTEN;         // 0x30 - Interrupt Enable
        _GPIO_INTMASK           GPIO_INTMASK;       // 0x34 - Interrupt Mask
        _GPIO_INTTYPE_LEVEL     GPIO_INTTYPE_LEVEL; // 0x38 - Interrupt Type
        _GPIO_INT_POLARITY      GPIO_INT_POLARITY;  // 0x3C - Interrupt Polarity
        _GPIO_INTSTATUS         GPIO_INTSTATUS;     // 0x40 - Interrupt Status
        _GPIO_RAW_INTSTATUS     GPIO_RAW_INTSTATUS; // 0x44 - Raw Interrupt Status
        _GPIO_DEBOUNCE          GPIO_DEBOUNCE;      // 0x48 - Debounce enable
        _GPIO_PORTA_EOI         GPIO_PORTA_EOI;     // 0x4C - Clear Interrupt
        _GPIO_EXT_PORTA         GPIO_EXT_PORTA;     // 0x50 - Port A External Port
        ULONG                   _reserved[0x0C];    // 0x54 - 0x5F
        _GPIO_LS_SYNC           GPIO_LS_SYNC;       // 0x60 - Synchronization Level
    } volatile FABRIC_GPIO, *PFABRIC_GPIO;

    //
    // FabricGpioControllerClass private data members.
    //

    // Handle to the device that can be used to map Fabric GPIO Controller registers
    // into user-mode address space.
    HANDLE m_hController;

    // Pointer to the object used to address the Fabric GPIO Controller registers after
    // they are mapped into this process' address space.
    PFABRIC_GPIO m_controller;

    //
    // FabricGpioControllerClass private methods.
    //

    // Method to map the Fabric GPIO Controller into this process' virtual address space.
    BOOL _mapController()
    {
        BOOL status = TRUE;
        BOOL error = ERROR_SUCCESS;
        PVOID baseAddress = nullptr;

        status = GetControllerBaseAddress(dmapGpioDeviceName,
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

    // Method to set a Fabric GPIO pin as an input.
    inline void _setPinInput(ULONG portBit)
    {
        _interlockedbittestandreset((LONG*)&m_controller->GPIO_SWPORTA_DDR.ALL_BITS, portBit);
    }

    // Method to set a Fabric GPIO pin as an output
    inline void _setPinOutput(ULONG portBit)
    {
        _interlockedbittestandset((LONG*)&m_controller->GPIO_SWPORTA_DDR.ALL_BITS, portBit);
    }
};

__declspec (selectany) FabricGpioControllerClass g_fabricGpio;

#endif  // _GPIO_CONTROLLER_H_