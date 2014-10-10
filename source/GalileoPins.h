// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _GALILEO_PINS_H_
#define _GALILEO_PINS_H_

#include <Windows.h>

#include "ArduinoCommon.h"
#include "GpioController.h"
#include "PCAL9535ASuppport.h"
#include "PCA9685Support.h"
#include "ExpanderDefs.h"

/// The class used to configure and use GPIO pins.
class GalileoPinsClass
{
public:
    GalileoPinsClass();

    virtual ~GalileoPinsClass()
    {
    }

    /// Struct for pin-specific attributes.
    /** This struct contains all the attributes needed to configure and use one
        of the I/O pins. */
    typedef struct {
        UCHAR gpioType;         ///< Fabric, Legacy Resume, Legacy Core, Expander
        UCHAR portBit;          ///< Which bit on the port is attached to this pin
        UCHAR pullupExp : 4;    ///< Number of I/O expander for pull-up control
        UCHAR pullupBit : 4;    ///< Bit of I/O expander for pull-up control
        UCHAR triStExp : 4;     ///< Number of I/O expander for tri-state control
        UCHAR triStBit : 4;     ///< Bit of I/O expander for tri-state control
        UCHAR muxA : 4;         ///< Number of first MUX for pin, if any
        UCHAR muxB : 4;         ///< Number of second MUX for pin, if any
        UCHAR digIoMuxA : 1;    ///< State of 1st MUX for digital I/O use of pin
        UCHAR digIoMuxB : 1;    ///< State of 2nd MUX for digital I/O use of pin
        UCHAR pwmMuxA : 1;      ///< State of 1st MUX for PWM use of pin
        UCHAR pwmMuxB : 1;      ///< State of 2nd MUX for PWM use of pin
        UCHAR anInMuxA : 1;     ///< State of 1st MUX for analog input use of pin
        UCHAR anInMuxB : 1;     ///< State of 2nd MUX for analog input use of pin
        UCHAR i2cMuxA : 1;      ///< State of 1st MUX for I2C use of pin
        UCHAR i2cMuxB : 1;      ///< State of 2nd MUX for I2C use of pin
        UCHAR spiMuxA : 1;      ///< State of 1st MUX for SPI use of pin
        UCHAR spiMuxB : 1;      ///< State of 2nd MUX for SPI use of pin
        UCHAR serMuxA : 1;      ///< State of 1st MUX for serial use of pin
        UCHAR serMuxB : 1;      ///< State of 1nd MUX for serial use of pin
        UCHAR triStIn : 1;      ///< Tri-state control bit state for input pin
        UCHAR _pad : 3;         ///< Pad to byte boundary
        UCHAR funcMask;         ///< Mask of functin types supported on the pin
    } PORT_ATTRIBUTES, *PPORT_ATTRIBUTES;
    
    /// Struct for mux-specific attributes.
    /** This struct contains all the attributes needed to use a mux. */
    typedef struct {
        UCHAR selectExp : 4;    ///< I/O Expander that drives the select signal
        UCHAR selectBit : 4;    ///< Bit of I/O Expander that drives the select signal
    } MUX_ATTRIBUTES, *PMUX_ATTRIBUTES;

    /// Struct for I/O Expander-specific attributes.
    /** This struce stores all the attributes needed to access an I/O Expander. */
    typedef struct {
        UCHAR Exp_Type;         ///< I/O Expander chip type
        UCHAR I2c_Address;      ///< I2C address of the I/O expander
    } EXP_ATTRIBUTES, *PEXP_ATTRIBUTES;

    /// Struct used to store function state for each pin.
    /**
    Struct used to track function configuration on a pin.  It holds the currently configured
    function and whether the function is allowed to be changed implicitely.
    */
    typedef struct {
        UCHAR currentFunction;  ///< Function the pin is currently configure for
        BOOLEAN locked;         ///< True locks the pin to current function, false allows change
    } PIN_FUNCTION, *PPIN_FUNCTION;

    /// Struct used to store PWM Channel information.
    /**
    This struct stores PWM Channel attributes for a GPIO pin.
    */
    typedef struct {
        UCHAR expander;         ///< I/O Expander that drives PWM for this pin
        UCHAR channel;          ///< PWM channel on the expander for PWM for this pin
    } PWM_CHANNEL, *PPWM_CHANNEL;

    /// Enum of function lock actions.
    const enum FUNC_LOCK_ACTION {
        NO_LOCK_CHANGE,         ///< Don't take any lock action
        LOCK_FUNCTION,          ///< Lock the pin to this function
        UNLOCK_FUNCTION         ///< Unlock the pin function
    };

    /// Method to set an I/O pin to a state (HIGH or LOW).
    BOOL _setPinState(ULONG pin, ULONG state);

    /// Method to read the state of an I/O pin.
    BOOL _getPinState(ULONG pin, ULONG & state);

    /// Method to set the direction of a pin (DIRECTION_IN or DIRECTION_OUT).
    BOOL _setPinMode(ULONG pin, ULONG mode, BOOL pullUp);

    /// Method to verify that a pin is configured for the desired function.
    BOOL _verifyPinFunction(ULONG pin, ULONG function, FUNC_LOCK_ACTION lockAction);

    /// Method to set the PWM duty cycle for a pin.
    BOOL _setPwmDutyCycle(ULONG pin, ULONG dutyCycle);

private:

    /// Method to configure an I/O Pin for one of the functions it suppports.
    BOOL _setPinFunction(ULONG pin, ULONG function);

    /// Pointer to the array of pin attributes.
    const PORT_ATTRIBUTES* m_PinAttributes;

    /// Pointer to the array of MUX attributes.
    const MUX_ATTRIBUTES* m_MuxAttributes;

    /// Pointer to the array of I/O Expander attributes.
    const EXP_ATTRIBUTES* m_ExpAttributes;

    /// Pointer to array of pin function tracking structures.
    const PPIN_FUNCTION m_PinFunctions;

    /// Pointer to array of PWM channels.
    const PWM_CHANNEL* m_PwmChannels;

    /// Method to configure an I/O Pin for Digital I/O use.
    BOOL _setPinDigitalIo(ULONG pin);

    /// Method to configure an I/O Pin for PWM use.
    BOOL _setPinPwm(ULONG pin);

    /// Method to configure an I/O Pin for Analog Input use.
    BOOL _setPinAnalogInput(ULONG pin);

    /// Method to configure an I/O Pin for I2C Bus use.
    BOOL _setPinI2c(ULONG pin);

    /// Method to configure an I/O Pin for SPI Bus use.
    BOOL _setPinSpi(ULONG pin);

    /// Method to configure an I/O Pin for Hardware Serial use.
    BOOL _setPinHwSerial(ULONG pin);

    /// Method to choose between the input driver and output driver for an I/O pin.
    BOOL _configurePinDrivers(ULONG pin, ULONG mode);

    /// Method to turn the pullup resistor on or off for an I/O pin.
    BOOL _configurePinPullup(ULONG pin, BOOL pullUp);

    /// Method to set a mux to a desired state.
    BOOL _setMux(ULONG mux, ULONG selection);

    /// Method to set the direction on an I/O Expander port pin.
    BOOL _setExpBitDirection(ULONG expNo, ULONG bitNo, ULONG directin);

    /// Method to set the state of an I/O Expander port pin.
    BOOL _setExpBitToState(ULONG expNo, ULONG bitNo, ULONG state);

    /// Method to test whether a pin number is in the valid range or not.
    inline BOOL _pinNumberIsValid(ULONG pin);
};

/// Global object used to configure and use the I/O pins.
__declspec (selectany) GalileoPinsClass g_pins;

#endif // _GALILEO_PINS_H_