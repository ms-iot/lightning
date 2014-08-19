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


class GalileoPinsClass
{
public:
    GalileoPinsClass();

    virtual ~GalileoPinsClass()
    {

    }

    // This struct stores all the pin-specific attributes needed to configure 
    // and use one of the I/O pins.
    typedef struct ABC {
        UCHAR gpioType;         // Fabric, Legacy Resume, Legacy Core, Expander
        UCHAR portBit;          // Which bit on the port is attached to this pin
        UCHAR pullupExp : 4;    // Number of I/O expander for pull-up control
        UCHAR pullupBit : 4;    // Bit of I/O expander for pull-up control
        UCHAR triStExp : 4;     // Number of I/O expander for tri-state control
        UCHAR triStBit : 4;     // Bit of I/O expander for tri-state control
        UCHAR muxA : 4;         // Number of first MUX for pin, if any
        UCHAR muxB : 4;         // Number of second MUX for pin, if any
        UCHAR digIoMuxA : 1;    // State of 1st MUX for digital I/O use of pin
        UCHAR digIoMuxB : 1;    // State of 2nd MUX for digital I/O use of pin
        UCHAR pwmMuxA : 1;      // State of 1st MUX for PWM use of pin
        UCHAR pwmMuxB : 1;      // State of 2nd MUX for PWM use of pin
        UCHAR anInMuxA : 1;     // State of 1st MUX for analog input use of pin
        UCHAR anInMuxB : 1;     // State of 2nd MUX for analog input use of pin
        UCHAR i2cMuxA : 1;      // State of 1st MUX for I2C use of pin
        UCHAR i2cMuxB : 1;      // State of 2nd MUX for I2C use of pin
        UCHAR spiMuxA : 1;      // State of 1st MUX for SPI use of pin
        UCHAR spiMuxB : 1;      // State of 2nd MUX for SPI use of pin
        UCHAR serMuxA : 1;      // State of 1st MUX for serial use of pin
        UCHAR serMuxB : 1;      // State of 1nd MUX for serial use of pin
        UCHAR triStIn : 1;      // Tri-state control bit state for input pin
        UCHAR _pad : 3;         // Pad to byte boundary
        UCHAR funcMask;         // Mask of functin types supported on the pin
    } PORT_ATTRIBUTES, *PPORT_ATTRIBUTES;

    // This struct stores the mux-specific attributes needed to use the mux.
    typedef struct {
        UCHAR selectExp : 4;    // I/O Expander that drives the select signal
        UCHAR selectBit : 4;    // Bit of I/O Expander that drives select
    } MUX_ATTRIBUTES, *PMUX_ATTRIBUTES;

    // This struct stores the I/O Expander-specific attributes needed to use it.
    typedef struct {
        UCHAR Exp_Type;         // I/O Expander chip type
        UCHAR I2c_Address;      // I2C address of the I/O expander
    } EXP_ATTRIBUTES, *PEXP_ATTRIBUTES;

    BOOL _setPinState(UCHAR pin, UCHAR state);
    BOOL _setPinMode(UCHAR pin, UCHAR mode, bool pullUp);
    BOOL _setPinFunction(UCHAR pin, UCHAR function);

private:
    // Pointer to the array of pin attributes.
    const PORT_ATTRIBUTES* m_PinAttributes;

    // Pointer to the array of MUX attributes.
    const MUX_ATTRIBUTES* m_MuxAttributes;

    // Pointer to the array of I/O Expander attributes.
    const EXP_ATTRIBUTES* m_ExpAttributes;

    BOOL _setPinDigitalIo(UCHAR pin);
    BOOL _setPinPwm(UCHAR pin);
    BOOL _setPinAnalogInput(UCHAR pin);
    BOOL _setPinI2c(UCHAR pin);
    BOOL _setPinSpi(UCHAR pin);
    BOOL _setPinHwSerial(UCHAR pin);

    BOOL _configurePinDrivers(UCHAR pin, UCHAR mode);
    BOOL _configurePinPullup(UCHAR pin, bool pullUp);

    BOOL _setMux(UCHAR mux, UCHAR selection);

    BOOL _setExpBitDirection(UCHAR expNo, UCHAR bitNo, UCHAR directin);
    BOOL _setExpBitToState(UCHAR expNo, UCHAR bitNo, UCHAR state);

    inline BOOL _pinNumberIsValid(UCHAR pin);

};

__declspec (selectany) GalileoPinsClass g_pins;

#endif // _GALILEO_PINS_H_