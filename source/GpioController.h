// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _GPIO_CONTROLLER_H_
#define _GPIO_CONTROLLER_H_

#include <Windows.h>
#include "ArduinoCommon.h"
#include "DmapSupport.h"
#include "quarklgpio.h"

/// Class used to interact with the Fabric GPIO hardware.
class FabricGpioControllerClass
{
public:
    /// Constructor.
    FabricGpioControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
        m_controller = nullptr;
    }

    /// Destructor.
    virtual ~FabricGpioControllerClass()
    {
        if (m_hController != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hController);
            m_hController = INVALID_HANDLE_VALUE;
        }
        m_controller = nullptr;
    }

    /// Method to map the Fabric GPIO controller registers if they are not already mapped.
    /**
    \return TRUE success, FALSE failure, GetLastError() provides the error code.
    */
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

    /// Method to set the state of a Fabric GPIO port bit.
    inline BOOL setPinState(ULONG portBit, ULONG state);

    /// Method to read the state of a Fabric GPIO bit.
    inline BOOL getPinState(ULONG portBit, ULONG & state);

    /// Method to set the direction (input or output) of a Fabric GPIO port bit.
    inline BOOL setPinDirection(ULONG portBit, ULONG mode);

    /// Method to get the direction (input or output) of a Fabric GPIO port bit.
    inline BOOL getPinDirection(ULONG portBit, ULONG & mode);

private:

    #pragma warning(push)
    #pragma warning(disable : 4201) // Ignore nameless struct/union warnings

    /// Port A Data Register.
    typedef union {
        struct {
            ULONG GPIO_SWPORTA_DR : 8;      ///< Port Data
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_SWPORTA_DR;

    /// Port A Data Direction Register.
    typedef union {
        struct {
            ULONG GPIO_SWPORTA_DDR : 8;     ///< Port Data Direction
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_SWPORTA_DDR;

    /// Interrupt Enable Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTEN : 8;           ///< Interrupt Enable
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTEN;

    /// Interrupt Mask Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTMASK : 8;         ///< Interrupt Mask
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTMASK;

    /// Interrupt Type Register for Port A.
    typedef union {
        struct {
            ULONG GPI_INTTYPE_LEVEL : 8;    ///< Interrupt Type
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTTYPE_LEVEL;

    /// Interrupt Polarity Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INT_POLARITY : 8;    ///< Interrupt Polarity
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INT_POLARITY;

    /// Interrupt Status Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_INTSTATUS : 8;       ///< Interrupt Status
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_INTSTATUS;

    /// Raw Interrupt Status Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_RAW_INSTATUS : 8;    ///< Raw Interrupt Status
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_RAW_INTSTATUS;

    /// Debounce Enable Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_DEBOUNCE : 8;        ///< Debounce Enable
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_DEBOUNCE;

    /// Clear Interrupt Register for Port A.
    typedef union {
        struct {
            ULONG GPIO_PORTA_EOI : 8;       ///< Clear Interrupt
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_PORTA_EOI;

    /// Port A External Port Register.
    typedef union {
        struct {
            ULONG GPIO_EXT_PORTA : 8;       ///< External Port
            ULONG _reserved : 24;
        };
        ULONG ALL_BITS;
    } _GPIO_EXT_PORTA;

    /// Register controls Synchronization of Level-Sensitive interrupts.
    typedef union {
        struct {
            ULONG GPIO_LS_SYNC : 1;         ///< Synchronization Level
            ULONG _reserved : 31;
        };
        ULONG ALL_BITS;
    } _GPIO_LS_SYNC;

    #pragma warning( pop )

    /// Layout of the Quark Fabric GPIO Controller registers in memory.
    typedef struct _FABRIC_GPIO {
        _GPIO_SWPORTA_DR        GPIO_SWPORTA_DR;    ///< 0x00 - Port A Data
        _GPIO_SWPORTA_DDR       GPIO_SWPORTA_DDR;   ///< 0x04 - Port A Data Direction
        ULONG                   _reserved1[0x0A];   ///< 0x08 - 0x2F
        _GPIO_INTEN             GPIO_INTEN;         ///< 0x30 - Interrupt Enable
        _GPIO_INTMASK           GPIO_INTMASK;       ///< 0x34 - Interrupt Mask
        _GPIO_INTTYPE_LEVEL     GPIO_INTTYPE_LEVEL; ///< 0x38 - Interrupt Type
        _GPIO_INT_POLARITY      GPIO_INT_POLARITY;  ///< 0x3C - Interrupt Polarity
        _GPIO_INTSTATUS         GPIO_INTSTATUS;     ///< 0x40 - Interrupt Status
        _GPIO_RAW_INTSTATUS     GPIO_RAW_INTSTATUS; ///< 0x44 - Raw Interrupt Status
        _GPIO_DEBOUNCE          GPIO_DEBOUNCE;      ///< 0x48 - Debounce enable
        _GPIO_PORTA_EOI         GPIO_PORTA_EOI;     ///< 0x4C - Clear Interrupt
        _GPIO_EXT_PORTA         GPIO_EXT_PORTA;     ///< 0x50 - Port A External Port
        ULONG                   _reserved[0x03];    ///< 0x54 - 0x5F
        _GPIO_LS_SYNC           GPIO_LS_SYNC;       ///< 0x60 - Synchronization Level
    } volatile FABRIC_GPIO, *PFABRIC_GPIO;

    //
    // FabricGpioControllerClass private data members.
    //

    /// Handle to the controller device.
    /**
    This handle can be used to map Fabric GPIO Controller registers in to user
    memory address space.
    */
    HANDLE m_hController;

    /// Pointer to the controller object in this process' address space.
    /**
    This controller object is used to access the Fabric GPIO regiseters after
    they are mapped into this process' virtual address space.
    */
    PFABRIC_GPIO m_controller;

    //
    // FabricGpioControllerClass private methods.
    //

    /// Method to map the Fabric GPIO Controller into this process' virtual address space.
    BOOL _mapController();

    /// Method to set a Fabric GPIO pin as an input.
    inline void _setPinInput(ULONG portBit)
    {
        // Clear the appropriate Data Direction Register bit.  This operation is atomic, 
        // and therefore thread-safe on a single core processor (such as the Quark X1000).
        _bittestandreset((LONG*)&m_controller->GPIO_SWPORTA_DDR.ALL_BITS, portBit);
    }

    /// Method to set a Fabric GPIO pin as an output
    inline void _setPinOutput(ULONG portBit)
    {
        // Set the appropriate Data Direction Register bit.
        _bittestandset((LONG*)&m_controller->GPIO_SWPORTA_DDR.ALL_BITS, portBit);
    }
};

/// The global object used to interact with the Fabric GPIO hardware.
__declspec (selectany) FabricGpioControllerClass g_fabricGpio;


/// Class used to interact with the Legacy GPIO hardware.
class LegacyGpioControllerClass
{
public:
    /// Constructor.
    LegacyGpioControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
    }

    /// Destructor.
    virtual ~LegacyGpioControllerClass()
    {
        if (m_hController != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hController);
            m_hController = INVALID_HANDLE_VALUE;
        }
    }

    /// Method to open the Legacy GPIO controller if it is not already open.
    /**
    \return TRUE success, FALSE failure, GetLastError() provides the error code.
    */
    inline BOOL openIfNeeded()
    {
        if (m_hController == INVALID_HANDLE_VALUE)
        {
            return _openController();
        }
        else
        {
            return TRUE;
        }
    }

    /// Method to set the state of a Legacy Core Well GPIO port bit.
    inline BOOL setCorePinState(ULONG portBit, ULONG state);

    /// Method to read the state of a Legacy Core Well GPIO bit.
    inline BOOL getCorePinState(ULONG portBit, ULONG & state);

    /// Method to set the direction (input or output) of a Legacy Core Well GPIO port bit.
    inline BOOL setCorePinDirection(ULONG portBit, ULONG mode);

    /// Method to get the direction (input or output) of a Legacy Core Well GPIO port bit.
    inline BOOL getCorePinDirection(ULONG portBit, ULONG & mode);

    /// Method to set the state of a Legacy Resume Well GPIO port bit.
    inline BOOL setResumePinState(ULONG portBit, ULONG state);

    /// Method to read the state of a Legacy Resume Well GPIO bit.
    inline BOOL getResumePinState(ULONG portBit, ULONG & state);

    /// Method to set the direction (input or output) of a Legacy Resume Well GPIO port bit.
    inline BOOL setResumePinDirection(ULONG portBit, ULONG mode);

    /// Method to get the direction (input or output) of a Legacy Resume Well GPIO port bit.
    inline BOOL getResumePinDirection(ULONG portBit, ULONG & mode);

private:

    //
    // LegacyGpioControllerClass private data members.
    //

    /// Handle to the controller device.
    /**
    This handle can be used perform I/O operations on the Legacy GPIO Controller.
    */
    HANDLE m_hController;

    //
    // LegacyGpioControllerClass private methods.
    //

    /// Method to open the Legacy GPIO Controller.
    BOOL _openController();

    /// Method to set a Legacy Core Well GPIO pin as an input.
    inline BOOL _setCorePinInput(ULONG portBit)
    {
        QUARKLGPIO_INPUT_BUFFER inp;
        DWORD bytesReturned;

        inp.Bank = QUARKLGPIO_BANK_COREWELL;
        inp.Mask = 0x1 << portBit;

        return DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_SET_DIR_INPUT,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
    }

    /// Method to set a Legacy Core Well GPIO pin as an output
    inline BOOL _setCorePinOutput(ULONG portBit)
    {
        QUARKLGPIO_INPUT_BUFFER inp;
        DWORD bytesReturned;

        inp.Bank = QUARKLGPIO_BANK_COREWELL;
        inp.Mask = 0x1 << portBit;

        return DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_SET_DIR_OUTPUT,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
    }

    /// Method to set a Legacy Resume Well GPIO pin as an input.
    inline BOOL _setResumePinInput(ULONG portBit)
    {
        QUARKLGPIO_INPUT_BUFFER inp;
        DWORD bytesReturned;

        inp.Bank = QUARKLGPIO_BANK_RESUMEWELL;
        inp.Mask = 0x1 << portBit;

        return DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_SET_DIR_INPUT,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
    }

    /// Method to set a Legacy Resume Well GPIO pin as an output
    inline BOOL _setResumePinOutput(ULONG portBit)
    {
        QUARKLGPIO_INPUT_BUFFER inp;
        DWORD bytesReturned;

        inp.Bank = QUARKLGPIO_BANK_RESUMEWELL;
        inp.Mask = 0x1 << portBit;

        return DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_SET_DIR_OUTPUT,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
    }
};

/// The global object used to interact with the Lagacy GPIO hardware.
__declspec (selectany) LegacyGpioControllerClass g_legacyGpio;

/**
This method assumes the caller has verified the input parameters.
\param[in] portBit The number of the bit to set. Range: 0-7.
\param[in] state The state to set on the port bit. 0 - low, 1 - high.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL FabricGpioControllerClass::setPinState(ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = mapIfNeeded();

    if (status)
    {
        if (state == 0)
        {
            _bittestandreset((LONG*)&m_controller->GPIO_SWPORTA_DR.ALL_BITS, portBit);
        }
        else
        {
            _bittestandset((LONG*)&m_controller->GPIO_SWPORTA_DR.ALL_BITS, portBit);
        }
    }

    return status;
}

/**
If the port bit is configured as in input, the state passed back is the state of the external
signal connnected to the bit.  If the port bit is configured as an output, the state passed
back is the state that was last written to the port bit.  This method assumes the caller has
verified the input parameter.
\param[in] portBit The number of the bit to set. Range: 0-7.
\param[out] state The state read from the port bit.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL FabricGpioControllerClass::getPinState(ULONG portBit, ULONG & state)
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

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-7.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL FabricGpioControllerClass::setPinDirection(ULONG portBit, ULONG mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = mapIfNeeded();
    if (!status) { error = GetLastError(); }

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

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-7.
\param[out] mode The mode of the bit, DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
inline BOOL FabricGpioControllerClass::getPinDirection(ULONG portBit, ULONG & mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = mapIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        if ((m_controller->GPIO_SWPORTA_DDR.ALL_BITS >> portBit) == 0)
        {
            mode = DIRECTION_IN;
        }
        else
        {
            mode = DIRECTION_OUT;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-7.
\param[in] state The state to set on the port bit. 0 - low, 1 - high.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL LegacyGpioControllerClass::setCorePinState(ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD ioCtl;
    DWORD bytesReturned;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        inp.Bank = QUARKLGPIO_BANK_COREWELL;
        inp.Mask = 0x1 << portBit;

        if (state == 0)
        {
            ioCtl = IOCTL_QUARKLGPIO_SET_PINS_LOW;
        }
        else
        {
            ioCtl = IOCTL_QUARKLGPIO_SET_PINS_HIGH;
        }

        status = DeviceIoControl(
            m_hController,
            ioCtl,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to read. Range: 0-7.
\param[out] state Set to the state of the input bit.  0 - LOW, 1 - HIGH.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL LegacyGpioControllerClass::getCorePinState(ULONG portBit, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD bytesReturned;
    DWORD portContents;

status = openIfNeeded();
if (!status) { error = GetLastError(); }

if (status)
{
    inp.Bank = QUARKLGPIO_BANK_COREWELL;
    inp.Mask = 0x1 << portBit;

    status = DeviceIoControl(
        m_hController,
        IOCTL_QUARKLGPIO_READ_PINS,
        &inp,
        sizeof(inp),
        &portContents,
        sizeof(portContents),
        &bytesReturned,
        nullptr);
    if (!status) { error = GetLastError(); }
}

if (status)
{
    state = (portContents >> portBit) & 0x01;
}

if (!status) { SetLastError(error); }
return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-1.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL LegacyGpioControllerClass::setCorePinDirection(ULONG portBit, ULONG mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        switch (mode)
        {
        case DIRECTION_IN:
            status = _setCorePinInput(portBit);
            if (!status) { error = GetLastError(); }
            break;
        case DIRECTION_OUT:
            status = _setCorePinOutput(portBit);
            if (!status) { error = GetLastError(); }
            break;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.  This method is largely
intended for testing use--it is more efficient to just set the desired direction rather
than checking first.
\param[in] portBit The number of the bit to modify. Range: 0-1.
\param[out] mode The mode of the bit, DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
inline BOOL LegacyGpioControllerClass::getCorePinDirection(ULONG portBit, ULONG & mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD readValue;
    DWORD bytesReturned;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {

        inp.Bank = QUARKLGPIO_BANK_COREWELL;
        inp.Mask = 0xFF;                // Not used, but if it were, we would want all 8 bits

        status = DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_GET_DIR,
            &inp,
            sizeof(inp),
            &readValue,
            sizeof(readValue),
            &bytesReturned,
            nullptr);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            if (((readValue >> portBit) & 0x01) == 1)
            {
                mode = DIRECTION_IN;
            }
            else
            {
                mode = DIRECTION_OUT;
            }
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.  This method is largely
intended for testing use--it is more efficient to just set the desired direction rather
than checking first.
\param[in] portBit The number of the bit to modify. Range: 0-5.
\param[out] mode The mode of the bit, DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL LegacyGpioControllerClass::getResumePinDirection(ULONG portBit, ULONG & mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD readValue;
    DWORD bytesReturned;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {

        inp.Bank = QUARKLGPIO_BANK_RESUMEWELL;
        inp.Mask = 0xFF;                // Not used, but if it were, we would want all 8 bits

        status = DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_GET_DIR,
            &inp,
            sizeof(inp),
            &readValue,
            sizeof(readValue),
            &bytesReturned,
            nullptr);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            if (((readValue >> portBit) & 0x01) == 1)
            {
                mode = DIRECTION_IN;
            }
            else
            {
                mode = DIRECTION_OUT;
            }
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-5.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
inline BOOL LegacyGpioControllerClass::setResumePinDirection(ULONG portBit, ULONG mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        switch (mode)
        {
        case DIRECTION_IN:
            status = _setResumePinInput(portBit);
            if (!status) { error = GetLastError(); }
            break;
        case DIRECTION_OUT:
            status = _setResumePinOutput(portBit);
            if (!status) { error = GetLastError(); }
            break;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to modify. Range: 0-7.
\param[in] state The state to set on the port bit. 0 - low, 1 - high.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
inline BOOL LegacyGpioControllerClass::setResumePinState(ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD ioCtl;
    DWORD bytesReturned;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        inp.Bank = QUARKLGPIO_BANK_RESUMEWELL;
        inp.Mask = 0x1 << portBit;

        if (state == 0)
        {
            ioCtl = IOCTL_QUARKLGPIO_SET_PINS_LOW;
        }
        else
        {
            ioCtl = IOCTL_QUARKLGPIO_SET_PINS_HIGH;
        }

        status = DeviceIoControl(
            m_hController,
            ioCtl,
            &inp,
            sizeof(inp),
            nullptr,
            0,
            &bytesReturned,
            nullptr);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method assumes the caller has checked the input parameters.
\param[in] portBit The number of the bit to read. Range: 0-7.
\param[out] state Set to the state of the input bit.  0 - LOW, 1 - HIGH.
\return TRUE success, FALSE failure, GetLastError() provides the error code.
*/
BOOL LegacyGpioControllerClass::getResumePinState(ULONG portBit, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    QUARKLGPIO_INPUT_BUFFER inp;
    DWORD bytesReturned;
    DWORD portContents;

    status = openIfNeeded();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        inp.Bank = QUARKLGPIO_BANK_RESUMEWELL;
        inp.Mask = 0x1 << portBit;

        status = DeviceIoControl(
            m_hController,
            IOCTL_QUARKLGPIO_READ_PINS,
            &inp,
            sizeof(inp),
            &portContents,
            sizeof(portContents),
            &bytesReturned,
            nullptr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        state = (portContents >> portBit) & 0x01;
    }

    if (!status) { SetLastError(error); }
    return status;
}

#endif  // _GPIO_CONTROLLER_H_