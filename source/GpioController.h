// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _GPIO_CONTROLLER_H_
#define _GPIO_CONTROLLER_H_

#include <Windows.h>
#include <functional>

#include "ErrorCodes.h"

#include "ArduinoCommon.h"
#include "DmapSupport.h"
#include "HiResTimer.h"
#include "concrt.h"
#include "GpioInterrupt.h"


#if defined(_M_IX86) || defined(_M_X64)
/// Class used to interact with the BayTrail Fabric GPIO hardware.
class BtFabricGpioControllerClass
{
public:
    /// Constructor.
    BtFabricGpioControllerClass()
    {
        m_hS0Controller = INVALID_HANDLE_VALUE;
        m_hS5Controller = INVALID_HANDLE_VALUE;
        m_s0Controller = nullptr;
        m_s5Controller = nullptr;
    }

    /// Destructor.
    virtual ~BtFabricGpioControllerClass()
    {
        DmapCloseController(m_hS0Controller);
        m_s0Controller = nullptr;

        DmapCloseController(m_hS5Controller);
        m_s5Controller = nullptr;

    }

    /// Method to map the S0 GPIO controller registers if they are not already mapped.
    /**
    \return HRESULT error or success code.
    */
    inline HRESULT mapS0IfNeeded()
    {
        HRESULT hr = S_OK;

        if (m_hS0Controller == INVALID_HANDLE_VALUE)
        {
            hr = _mapS0Controller();
        }

        return hr;
    }

    /// Method to map the S5 GPIO controller registers if they are not already mapped.
    /**
    \return HRESULT error or success code.
    */
    inline HRESULT mapS5IfNeeded()
    {
        HRESULT hr = S_OK;

        if (m_hS5Controller == INVALID_HANDLE_VALUE)
        {
            hr = _mapS5Controller();
        }

        return hr;
    }

    /// Method to set the state of an S0 GPIO port bit.
    inline HRESULT setS0PinState(ULONG gpioNo, ULONG state);

    /// Method to set the state of an S5 GPIO port bit.
    inline HRESULT setS5PinState(ULONG gpioNo, ULONG state);

    /// Method to read the state of an S0 GPIO bit.
    inline HRESULT getS0PinState(ULONG gpioNo, ULONG & state);

    /// Method to read the state of an S5 GPIO bit.
    inline HRESULT getS5PinState(ULONG gpioNo, ULONG & state);

    /// Method to set the direction (input or output) of an S0 GPIO port bit.
    inline HRESULT setS0PinDirection(ULONG gpioNo, ULONG mode);

    /// Method to set the direction (input or output) of an S5 GPIO port bit.
    inline HRESULT setS5PinDirection(ULONG gpioNo, ULONG mode);

    /// Method to set the function (mux state) of an S0 GPIO port bit.
    inline HRESULT setS0PinFunction(ULONG gpioNo, ULONG function);

    /// Method to set the function (mux state) of an S5 GPIO port bit.
    inline HRESULT setS5PinFunction(ULONG gpioNo, ULONG function);

    /// Method to attach to an interrupt on an S0 GPIO port bit.
    HRESULT attachS0Interrupt(ULONG pin, std::function<void(void)> func, ULONG mode);

    /// Method to attach to an interrupt on an S0 GPIO port bit.
    HRESULT attachS0InterruptEx(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode);

    /// Method to attach to an interrupt on an S0 GPIO port bit.
    HRESULT attachS0InterruptContext(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode);

    /// Method to attach to an interrupt on an S5 GPIO port bit.
    HRESULT attachS5Interrupt(ULONG pin, std::function<void(void)> func, ULONG mode);

    /// Method to attach to an interrupt on an S5 GPIO port bit.
    HRESULT attachS5InterruptEx(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode);

    /// Method to attach to an interrupt on an S5 GPIO port bit.
    HRESULT attachS5InterruptContext(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode);

    /// Method to detach an interrupt for an S0 GPIO port bit.
    HRESULT detachS0Interrupt(ULONG pin);

    /// Method to detach an interrupt for an S5 GPIO port bit.
    HRESULT detachS5Interrupt(ULONG pin);

    /// Method to enable delivery of GPIO interrupts.
    inline HRESULT enableInterrupts()
    {
        return m_gpioInterrupts.enableInterrupts();
    }

    /// Method to disable delivery of GPIO interrupts.
    inline HRESULT disableInterrupts()
    {
        return m_gpioInterrupts.disableInterrupts();
    }

private:

#pragma warning(push)
#pragma warning(disable : 4201) // Ignore nameless struct/union warnings

    /// Pad Configuration Register.  Active high (1 enables) unless noted.
    typedef union {
        struct {
            ULONG FUNC_PIN_MUX : 3;         ///< Functional Pin Muxing
            ULONG _rsv0 : 1;
            ULONG IDYNWK2KEN : 1;           ///< Reduce weak 2k contention current
            ULONG _rsv1 : 2;
            ULONG PULL_ASSIGN : 2;          ///< Pull assignment: 0 - None, 1 - Up, 2 - Down
            ULONG PULL_STR : 2;             ///< Pull strength: 0 - 2k, 1 - 10k, 2 - 20k, 3 - 40k
            ULONG BYPASS_FLOP : 1;          ///< Bypass pad I/O flops: 0 - Flop enabled if exists
            ULONG _rsv2 : 1;
            ULONG IHYSCTL : 2;              ///< Hysteresis control
            ULONG IHYSENB : 1;              ///< Hysteresis enable, active low
            ULONG FAST_CLKGATE : 1;         ///< 1 enables the glitch filter fast clock
            ULONG SLOW_CLKGATE : 1;         ///< 1 enables the glitch filter slow clock
            ULONG FILTER_SLOW : 1;          ///< Use RTC clock for unglitch filter
            ULONG FILTER_EN : 1;            ///< Enable the glitch filter
            ULONG DEBOUNCE : 1;             ///< Enable debouncer (uses community debounce time)
            ULONG _rsv3 : 2;
            ULONG STRAP_VAL : 1;            ///< Reflect strap pin value even if overriden
            ULONG GD_LEVEL : 1;             ///< 1 - Use level IRQ, 0 - Edge triggered IRQ
            ULONG GD_TPE : 1;               ///< 1 - Enable positive edge/level detection
            ULONG GD_TNE : 1;               ///< 1 - Enable negative edge/level detection
            ULONG DIRECT_IRQ_EN : 1;        ///< Enable direct wire interrupt, not shared.
            ULONG I25COMP : 1;              ///< Enable 25 ohm compensation of hflvt buffers
            ULONG DISABLE_SECOND_MASK : 1;  ///< Disable second mask when PB_CONFIG ALL_FUNC_MASK used
            ULONG _rsv4 : 1;
            ULONG IODEN : 1;                ///< Enable open drain.
        };
        ULONG ALL_BITS;
    } _PCONF0;

    /// Delay Line Multiplexer Register.
    typedef union {
        struct {
            ULONG DLL_STD_MUX : 5;         ///< Delay standard mux
            ULONG DLL_HGH_MUX : 5;         ///< Delay high mux
            ULONG DLL_DDR_MUX : 5;         ///< Delay ddr mux
            ULONG DLL_CF_OD : 1;           ///< Cf values, software override enable
            ULONG _rsv : 16;
        };
        ULONG ALL_BITS;
    } _PCONF1;

    /// Pad Value Register.
    typedef union {
        struct {
            ULONG PAD_VAL : 1;             ///< Value read from or written to the I/O pad
            ULONG IOUTENB : 1;             ///< Output enable, active low
            ULONG IINENB : 1;              ///< Input enable, active low
            ULONG FUNC_C_VAL : 15;         ///< C value for function delay
            ULONG FUNC_F_VAL : 4;          ///< F value for function delay
            ULONG _rsv : 10;
        };
        ULONG ALL_BITS;
    } _PAD_VAL;

#pragma warning( pop )

    /// Layout of the BayTrail GPIO Controller registers in memory for one pad.
    typedef struct _GPIO_PAD {
        _PCONF0   PCONF0;          ///< 0x00 - Pad Configuration
        _PCONF1   PCONF1;          ///< 0x04 - Delay Line Multiplexer
        _PAD_VAL  PAD_VAL;         ///< 0x08 - Pad Value
        ULONG     _reserved;       ///< 0x0C - 4 ULONG address space per register set
    } volatile GPIO_PAD, *PGPIO_PAD;

    //
    // BtFabricGpioControllerClass private data members.
    //

    /// Handle to the controller device for GPIOs active only in S0 state.
    /**
    This handle can be used to map the S0 GPIO Controller registers in to user
    memory address space.
    */
    HANDLE m_hS0Controller;

    /// Handle to the controller device for GPIOs active in S5 state.
    /**
    This handle can be used to map the S5 GPIO Controller registers in to user
    memory address space.
    */
    HANDLE m_hS5Controller;

    /// Pointer to the S0 GPIO controller object in this process' address space.
    /**
    This controller object is used to access the S0 GPIO registers after
    they are mapped into this process' virtual address space.
    */
    PGPIO_PAD m_s0Controller;

    /// Pointer to the S5 GPIO controller object in this process' address space.
    /**
    This controller object is used to access the S0 GPIO registers after
    they are mapped into this process' virtual address space.
    */
    PGPIO_PAD m_s5Controller;

    /// Object used to control and receive GPIO interrupts.
    GpioInterruptsClass m_gpioInterrupts;

    //
    // BtFabricGpioControllerClass private methods.
    //

    /// Method to map the S0 GPIO Controller into this process' virtual address space.
    HRESULT _mapS0Controller();

    /// Method to map the S5 GPIO Controller into this process' virtual address space.
    HRESULT _mapS5Controller();

    /// Method to set an S0 GPIO pin as an input.
    inline void _setS0PinInput(ULONG gpioNo);

    /// Method to set an S5 GPIO pin as an input.
    inline void _setS5PinInput(ULONG gpioNo);

    /// Method to set an S0 GPIO pin as an output
    inline void _setS0PinOutput(ULONG gpioNo);

    /// Method to set an S5 GPIO pin as an output
    inline void _setS5PinOutput(ULONG gpioNo);
};

/// The global object used to interact with the BayTrail Fabric GPIO hardware.
LIGHTNING_DLL_API extern BtFabricGpioControllerClass g_btFabricGpio;

#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_ARM)
/// Class used to interact with the PI2 BCM2836 GPIO hardware.
class BcmGpioControllerClass
{
public:
    /// Constructor.
    BcmGpioControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
        m_registers = nullptr;
    }

    /// Destructor.
    virtual ~BcmGpioControllerClass()
    {
        DmapCloseController(m_hController);
        m_registers = nullptr;
    }

    /// Method to map the BCM2836 GPIO controller registers if they are not already mapped.
    /**
    \return HRESULT error or success code.
    */
    LIGHTNING_DLL_API HRESULT mapIfNeeded();

    /// Method to set the state of a GPIO port bit.
    inline HRESULT setPinState(ULONG gpioNo, ULONG state);

    /// Method to read the state of a GPIO bit.
    inline HRESULT getPinState(ULONG gpioNo, ULONG & state);

    /// Method to set the direction (input or output) of a GPIO port bit.
    inline HRESULT setPinDirection(ULONG gpioNo, ULONG mode);

    /// Method to set the function (mux state) of a GPIO port bit.
    inline HRESULT setPinFunction(ULONG gpioNo, ULONG function);

    /// Method to turn pin pullup on or off.
    inline HRESULT setPinPullup(ULONG gpioNo, BOOL pullup);

    /// Method to attach to an interrupt on a GPIO port bit.
    HRESULT attachInterrupt(ULONG pin, std::function<void(void)> func, ULONG mode);

    /// Method to attach to an interrupt on a GPIO port bit, with information return.
    HRESULT attachInterruptEx(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, ULONG mode);

    /// Method to attach to an interrupt on a GPIO port bit, with information return and context.
    HRESULT attachInterruptContext(ULONG pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, PVOID context, ULONG mode);

    /// Method to detach an interrupt for a GPIO port bit.
    HRESULT detachInterrupt(ULONG pin);

    /// Method to enable delivery of GPIO interrupts.
    inline HRESULT enableInterrupts()
    {
        return m_gpioInterrupts.enableInterrupts();
    }

    /// Method to disable delivery of GPIO interrupts.
    inline HRESULT disableInterrupts()
    {
        return m_gpioInterrupts.disableInterrupts();
    }

private:

    // Value to write to GPPUD to turn pullup/down off for pins.
    const ULONG pullupOff = 0;

    // Value to write to GPPUD to turn pullup on for a pin.
    const ULONG pullupOn = 2;

    /// Layout of the BCM2836 GPIO Controller registers in memory.
    typedef struct _BCM_GPIO {
        ULONG   GPFSELN[6];         ///< 0x00-0x17 - Function select GPIO 00-53
        ULONG   _rsv01;             //   0x18
        ULONG   GPSET0;             ///< 0x1C - Output Set GPIO 00-31
        ULONG   GPSET1;             ///< 0x20 - Output Set GPIO 32-53
        ULONG   _rsv02;             //   0x24
        ULONG   GPCLR0;             ///< 0x28 - Output Clear GPIO 00-31
        ULONG   GPCLR1;             ///< 0x2C - Output Clear GPIO 32-53
        ULONG   _rsv03;             //   0x30
        ULONG   GPLEV0;             ///< 0x34 - Level GPIO 00-31
        ULONG   GPLEV1;             ///< 0x38 - Level GPIO 32-53
        ULONG   _rsv04;             //   0x3C
        ULONG   GPEDS0;             ///< 0x40 - Event Detect Status GPIO 00-31
        ULONG   GPEDS1;             ///< 0x44 - Event Detect Status GPIO 32-53
        ULONG   _rsv05;             //   0x48
        ULONG   GPREN0;             ///< 0x4C - Rising Edge Detect Enable GPIO 00-31
        ULONG   GPREN1;             ///< 0x50 - Rising Edge Detect Enable GPIO 32-53
        ULONG   _rsv06;             //   0x54
        ULONG   GPFEN0;             ///< 0x58 - Falling Edge Detect Enable GPIO 00-31
        ULONG   GPFEN1;             ///< 0x5C - Falling Edge Detect Enable GPIO 32-53
        ULONG   _rsv07;             //   0x60
        ULONG   GPHEN0;             ///< 0x64 - High Detect Enable GPIO 00-31
        ULONG   GPHEN1;             ///< 0x68 - High Detect Enable GPIO 32-53
        ULONG   _rsv08;             //   0x6C
        ULONG   GPLEN0;             ///< 0x70 - Low Detect Enable GPIO 00-31
        ULONG   GPLEN1;             ///< 0x74 - Low Detect Enable GPIO 32-53
        ULONG   _rsv09;             //   0x78
        ULONG   GPAREN0;            ///< 0x7C - Async Rising Edge Detect GPIO 00-31
        ULONG   GPAREN1;            ///< 0x80 - Async Rising Edge Detect GPIO 32-53
        ULONG   _rsv10;             //   0x84
        ULONG   GPAFEN0;            ///< 0x88 - Async Falling Edge Detect GPIO 00-31
        ULONG   GPAFEN1;            ///< 0x8C - Async Falling Edge Detect GPIO 32-53
        ULONG   _rsv11;             //   0x90
        ULONG   GPPUD;              ///< 0x94 - GPIO Pin Pull-up/down Enable
        ULONG   GPPUDCLK0;          ///< 0x98 - Pull-up/down Enable Clock GPIO 00-31
        ULONG   GPPUDCLK1;          ///< 0x9C - Pull-up/down Enable Clock GPIO 32-53
        ULONG   _rsv12[4];          //   0xA0
        ULONG   _Test;              //   0xB0
    } volatile BCM_GPIO, *PBCM_GPIO;

    //
    // BcmGpioControllerClass private data members.
    //

    /// Handle to the controller device for GPIOs.
    /**
    This handle can be used to refer to the opened GPIO Controller to perform such 
    actions as closing or locking the controller.
    */
    HANDLE m_hController;

    /// Pointer to the GPIO controller object in this process' address space.
    /**
    This controller object is used to access the GPIO registers after
    they are mapped into this process' virtual address space.
    */
    PBCM_GPIO m_registers;

    /// Object used to control and receive GPIO interrupts.
    GpioInterruptsClass m_gpioInterrupts;

    //
    // BcmGpioControllerClass private methods.
    //

    /// Method to map the Controller into this process' virtual address space.
    HRESULT _mapController();

};

/// The global object used to interact with the BayTrail Fabric GPIO hardware.
LIGHTNING_DLL_API extern BcmGpioControllerClass g_bcmGpio;

#endif // defined(_M_ARM)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S0 GPIO number of the pad to set. Range: 0-127.
\param[in] state State to set the pad to. 0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS0PinState(ULONG gpioNo, ULONG state)
{
    HRESULT hr = mapS0IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PAD_VAL padVal;
        padVal.ALL_BITS = m_s0Controller[gpioNo].PAD_VAL.ALL_BITS;
        if (state == 0)
        {
            padVal.PAD_VAL = 0;
        }
        else
        {
            padVal.PAD_VAL = 1;
        }
        m_s0Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S5 GPIO number of the pad to set. Range: 0-59.
\param[in] state State to set the pad to. 0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS5PinState(ULONG gpioNo, ULONG state)
{
    HRESULT hr = mapS5IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PAD_VAL padVal;
        padVal.ALL_BITS = m_s5Controller[gpioNo].PAD_VAL.ALL_BITS;
        if (state == 0)
        {
            padVal.PAD_VAL = 0;
        }
        else
        {
            padVal.PAD_VAL = 1;
        }
        m_s5Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S0 GPIO number of the pad to read. Range: 0-127.
\param[out] state Set to the state of the input bit.  0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::getS0PinState(ULONG gpioNo, ULONG & state)
{
    HRESULT hr = mapS0IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PAD_VAL padVal;
        padVal.ALL_BITS = m_s0Controller[gpioNo].PAD_VAL.ALL_BITS;
        state = padVal.PAD_VAL;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S5 GPIO number of the pad to read. Range: 0-59.
\param[out] state Set to the state of the input bit.  0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::getS5PinState(ULONG gpioNo, ULONG & state)
{
    HRESULT hr = mapS5IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PAD_VAL padVal;
        padVal.ALL_BITS = m_s5Controller[gpioNo].PAD_VAL.ALL_BITS;
        state = padVal.PAD_VAL;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S0 GPIO number of the pad to configure. Range: 0-127.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS0PinDirection(ULONG gpioNo, ULONG mode)
{
    HRESULT hr = S_OK;
    
    hr = mapS0IfNeeded();

    if (SUCCEEDED(hr))
    {
        switch (mode)
        {
        case DIRECTION_IN:
            _setS0PinInput(gpioNo);
            break;
        case DIRECTION_OUT:
            _setS0PinOutput(gpioNo);
            break;
        }
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S5 GPIO number of the pad to configure. Range: 0-59.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS5PinDirection(ULONG gpioNo, ULONG mode)
{
    HRESULT hr = S_OK;
    
    hr = mapS5IfNeeded();

    if (SUCCEEDED(hr))
    {
        switch (mode)
        {
        case DIRECTION_IN:
            _setS5PinInput(gpioNo);
            break;
        case DIRECTION_OUT:
            _setS5PinOutput(gpioNo);
            break;
        }
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S0 GPIO number of the pad to configure.  Range: 0-127.
\param[in] function The function to set for the pin.  Range: 0-7 (only 0-1 used here).
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS0PinFunction(ULONG gpioNo, ULONG function)
{
    HRESULT hr = S_OK;
    
    hr = mapS0IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PCONF0 padConfig;
        padConfig.ALL_BITS = m_s0Controller[gpioNo].PCONF0.ALL_BITS;
        padConfig.FUNC_PIN_MUX = function;
        m_s0Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The S5 GPIO number of the pad to configure.  Range: 0-59.
\param[in] function The function to set for the pin.  Range: 0-7 (only 0-1 used here).
\return HRESULT error or success code.
*/
inline HRESULT BtFabricGpioControllerClass::setS5PinFunction(ULONG gpioNo, ULONG function)
{
    HRESULT hr = S_OK;
    
    hr = mapS5IfNeeded();

    if (SUCCEEDED(hr))
    {
        _PCONF0 padConfig;
        padConfig.ALL_BITS = m_s5Controller[gpioNo].PCONF0.ALL_BITS;
        padConfig.FUNC_PIN_MUX = function;
        m_s5Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;
    }

    return hr;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This routine disables the output latch for the pad, disables pin output and
enables pin input.
\param[in] gpioNo The S0 GPIO number of the pad to configure. Range: 0-127.
*/
inline void BtFabricGpioControllerClass::_setS0PinInput(ULONG gpioNo)
{
    _PCONF0 padConfig;
    padConfig.ALL_BITS = m_s0Controller[gpioNo].PCONF0.ALL_BITS;
    padConfig.BYPASS_FLOP = 1;         // Disable flop
    padConfig.PULL_ASSIGN = 0;         // Disable pull-up
    m_s0Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;

    _PAD_VAL padVal;
    padVal.ALL_BITS = m_s0Controller[gpioNo].PAD_VAL.ALL_BITS;
    padVal.IINENB = 0;                 // Enable pad for input
    padVal.IOUTENB = 1;                // Disable pad for output
    m_s0Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This routine disables the output latch for the pad, disables pin output and
enables pin input.
\param[in] gpioNo The S5 GPIO number of the pad to configure. Range: 0-127.
*/
inline void BtFabricGpioControllerClass::_setS5PinInput(ULONG gpioNo)
{
    _PCONF0 padConfig;
    padConfig.ALL_BITS = m_s5Controller[gpioNo].PCONF0.ALL_BITS;
    padConfig.BYPASS_FLOP = 1;         // Disable flop
    padConfig.PULL_ASSIGN = 0;         // No pull resistor
    m_s5Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;

    _PAD_VAL padVal;
    padVal.ALL_BITS = m_s5Controller[gpioNo].PAD_VAL.ALL_BITS;
    padVal.IINENB = 0;                 // Enable pad for input
    padVal.IOUTENB = 1;                // Disable pad for output
    m_s5Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This routine enables the output latch for the pad, disables pull-ups on
the pad, disables pin input and enables pin output.
\param[in] gpioNo The S0 GPIO number of the pad to configure. Range: 0-127.
*/
inline void BtFabricGpioControllerClass::_setS0PinOutput(ULONG gpioNo)
{
    _PCONF0 padConfig;
    padConfig.ALL_BITS = m_s0Controller[gpioNo].PCONF0.ALL_BITS;
    padConfig.BYPASS_FLOP = 0;         // Enable flop
    padConfig.PULL_ASSIGN = 0;         // No pull resistor
    padConfig.FUNC_PIN_MUX = 0;        // Mux function 0 (GPIO)
    m_s0Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;

    _PAD_VAL padVal;
    padVal.ALL_BITS = m_s0Controller[gpioNo].PAD_VAL.ALL_BITS;
    padVal.IOUTENB = 0;                // Enable pad for output
    padVal.IINENB = 1;                 // Disable pad for input
    m_s0Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
}
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_IX86) || defined(_M_X64)
/**
This routine enables the output latch for the pad, disables pull-ups on
the pad, disables pin input and enables pin output.
\param[in] gpioNo The S5 GPIO number of the pad to configure. Range: 0-127.
*/
inline void BtFabricGpioControllerClass::_setS5PinOutput(ULONG gpioNo)
{
    _PCONF0 padConfig;
    padConfig.ALL_BITS = m_s5Controller[gpioNo].PCONF0.ALL_BITS;
    padConfig.BYPASS_FLOP = 0;         // Enable flop
    padConfig.PULL_ASSIGN = 0;         // No pull resistor
    m_s5Controller[gpioNo].PCONF0.ALL_BITS = padConfig.ALL_BITS;

    _PAD_VAL padVal;
    padVal.ALL_BITS = m_s5Controller[gpioNo].PAD_VAL.ALL_BITS;
    padVal.IOUTENB = 0;                // Enable pad for output
    padVal.IINENB = 1;                 // Disable pad for input
    m_s5Controller[gpioNo].PAD_VAL.ALL_BITS = padVal.ALL_BITS;
}
#endif // defined(_M_IX86) || defined(_M_X64)




#if defined(_M_ARM)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The GPIO number of the pad to set. Range: 0-31.
\param[in] state State to set the pad to. 0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BcmGpioControllerClass::setPinState(ULONG gpioNo, ULONG state)
{
    HRESULT hr = mapIfNeeded();
    ULONG bitMask;

    if (SUCCEEDED(hr))
    {
        if (gpioNo < 32)
        {
            bitMask = 1 << gpioNo;
            if (state == 0)
            {
                m_registers->GPCLR0 = bitMask;
            }
            else
            {
                m_registers->GPSET0 = bitMask;
            }
        }
        else
        {
            bitMask = 1 << (gpioNo - 32);
            if (state == 0)
            {
                m_registers->GPCLR1 = bitMask;
            }
            else
            {
                m_registers->GPSET1 = bitMask;
            }
        }
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The GPIO number of the pad to read. Range: 0-31.
\param[out] state Set to the state of the input bit.  0 - LOW, 1 - HIGH.
\return HRESULT error or success code.
*/
inline HRESULT BcmGpioControllerClass::getPinState(ULONG gpioNo, ULONG & state)
{
    HRESULT hr = mapIfNeeded();

    if (SUCCEEDED(hr))
    {
        if (gpioNo < 32)
        {
            state = ((m_registers->GPLEV0) >> gpioNo) & 1;
        }
        else
        {
            state = ((m_registers->GPLEV1) >> (gpioNo - 32)) & 1;
        }
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/**
This method assumes the caller has checked the input parameters.  This method has 
the side effect of setting the pin to GPIO use (since the BCM GPIO controller uses 
the same bits to control both function and direction).
\param[in] gpioNo The GPIO number of the pad to configure. Range: 0-53.
\param[in] mode The mode to set for the bit.  Range: DIRECTION_IN or DIRECTION_OUT
\return HRESULT error or success code.
*/
inline HRESULT BcmGpioControllerClass::setPinDirection(ULONG gpioNo, ULONG mode)
{
    HRESULT hr = S_OK;
    ULONG funcSelData = 0;

    hr = mapIfNeeded();

    if (SUCCEEDED(hr))
    {
        hr = GetControllerLock(m_hController);
    }

    if (SUCCEEDED(hr))
    {
        // Each GPIO has a 3-bit function field (000b selects input, 001b output, etc.) 
        // and there are 10 such fields in each 32-bit function select register.

        funcSelData = m_registers->GPFSELN[gpioNo / 10];   // Read function register data

        funcSelData &= ~(0x07 << ((gpioNo % 10) * 3));      // Clear bits for GPIO (make input)

        if (mode == DIRECTION_OUT)                          // If GPIO should be output:
        {
            funcSelData |= (0x01 << ((gpioNo % 10) * 3));   // Set one bit for GPIO (make output)
        }

        m_registers->GPFSELN[gpioNo / 10] = funcSelData;   // Write function register data back

        ReleaseControllerLock(m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The GPIO number of the pad to configure.  Range: 0-53.
\param[in] function The function to set for the pin.  Range: 0-1.  Function 0 is GPIO, and
function 1 is the non-GPIO use of the pin (called "alternate function 0" in the datasheet).
\return HRESULT error or success code.
*/
inline HRESULT BcmGpioControllerClass::setPinFunction(ULONG gpioNo, ULONG function)
{
    HRESULT hr = S_OK;
    ULONG funcSelData = 0;

    hr = mapIfNeeded();

    if (SUCCEEDED(hr))
    {
        hr = GetControllerLock(m_hController);
    }

    if (SUCCEEDED(hr))
    {
        // Each GPIO has a 3-bit function field (000b selects input, 001b output, etc.) 
        // and there are 10 such fields in each 32-bit function select register.

        funcSelData = m_registers->GPFSELN[gpioNo / 10];   // Read function register data

        // If the function is already set to GPIO, and the new function is GPIO, leave it 
        // alone (so we don't change the pin direction trying to set the pin function).
        if (((funcSelData & (0x06 << ((gpioNo % 10) * 3))) != 0) || (function != 0))
        {
            funcSelData &= ~(0x07 << ((gpioNo % 10) * 3));      // Clear bits for (make GPIO input)

            if (function == 1)                                  // If alternate function 0 is wanted:
            {
                funcSelData |= (0x04 << ((gpioNo % 10) * 3));   // Set function code to 100b
            }

            m_registers->GPFSELN[gpioNo / 10] = funcSelData;   // Write function register data back
        }

        ReleaseControllerLock(m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#if defined(_M_ARM)
/**
This method assumes the caller has checked the input parameters.
\param[in] gpioNo The GPIO number of the pad to configure.  Range: 0-53.
\param[in] pullup TRUE to turn pullup on for this pin, FALSE to turn it off.
\return HRESULT error or success code.
*/
inline HRESULT BcmGpioControllerClass::setPinPullup(ULONG gpioNo, BOOL pullup)
{
    HRESULT hr = S_OK;
    ULONG gpioPull = 0;
    HiResTimerClass timer;


    hr = mapIfNeeded();

    if (SUCCEEDED(hr))
    {
        hr = GetControllerLock(m_hController);
    }

    if (SUCCEEDED(hr))
    {
        if (pullup)
        {
            gpioPull = pullupOn;
        }
        else
        {
            gpioPull = pullupOff;
        }

        //
        // The sequence to set pullup/down for a pin is:
        // 1) Write desired state to GPPUD
        // 2) Wait 150 cycles
        // 3) Write clock mask to GPPUDCLK0/1 with bits set that correspond to pins to be configured
        // 4) Wait 150 cycles
        // 5) Write to GPPUD to remove state
        // 6) Write to GPPUDCLK0/1 to remove clock bits
        //
        // 150 cycles is 0.25 microseconds with a cpu clock of 600 Mhz.
        //
        m_registers->GPPUD = gpioPull;         // 1)

        timer.StartTimeout(1);
        while (!timer.TimeIsUp());              // 2)
        
        m_registers->GPPUDCLK0 = 1 << gpioNo;
        m_registers->GPPUDCLK1 = 0;            // 3)
        
        timer.StartTimeout(1);
        while (!timer.TimeIsUp());              // 4)
        
        m_registers->GPPUD = 0;                // 5)
        
        m_registers->GPPUDCLK0 = 0;
        m_registers->GPPUDCLK1 = 0;            // 6)

        ReleaseControllerLock(m_hController);
    }

    return hr;
}
#endif // defined(_M_ARM)

#endif  // _GPIO_CONTROLLER_H_