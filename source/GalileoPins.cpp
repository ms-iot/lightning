// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include <Windows.h>
#include "GalileoPins.h"
// GPIO type values.
const UCHAR GPIO_FABRIC = 1;
const UCHAR GPIO_LEGRES = 2;
const UCHAR GPIO_LEGCOR = 3;
const UCHAR GPIO_EXP1 = 4;
const UCHAR GPIO_EXP2 = 5;

// GPIO pin driver selection values.
const UCHAR GPIO_INPUT_DRIVER_SELECT = 1;
const UCHAR GPIO_OUTPUT_DRIVER_SELECT = 0;

// I/O Expander name values.
const UCHAR EXP0       =  0;    // I/O Expander 0
const UCHAR EXP1       =  1;    // I/O Expander 1
const UCHAR EXP2       =  2;    // I/O Expander 2
const UCHAR PWM        =  3;    // PWM used as I/O Expander
const UCHAR NUM_IO_EXP =  4;    // Number of I/O Expanders present
const UCHAR NO_X       = 15;    // No I/O Expander

// I/O Expander types.
const UCHAR PCAL9535A = 0;      // I/O Expander chip
const UCHAR PCA9685 = 1;        // PWM chip
const UCHAR NUM_EXP_TYPSES = 2; // Number of I/O Expanders types

// PWM chip bit values.
const UCHAR LED0  =  0;
const UCHAR LED1  =  1;
const UCHAR LED2  =  2;
const UCHAR LED3  =  3;
const UCHAR LED4  =  4;
const UCHAR LED5  =  5;
const UCHAR LED6  =  6;
const UCHAR LED7  =  7;
const UCHAR LED8  =  8;
const UCHAR LED9  =  9;
const UCHAR LED10 = 10;
const UCHAR LED11 = 11;
const UCHAR LED12 = 12;
const UCHAR LED13 = 13;
const UCHAR LED14 = 14;
const UCHAR LED15 = 15;

// MUX name values.
const UCHAR MUX0      =  0;
const UCHAR MUX1      =  1;
const UCHAR MUX2      =  2;
const UCHAR MUX3      =  3;
const UCHAR MUX4      =  4;
const UCHAR MUX5      =  5;
const UCHAR MUX6      =  6;
const UCHAR MUX7      =  7;
const UCHAR MUX8      =  8;
const UCHAR MUX9      =  9;
const UCHAR MUX10     = 10;
const UCHAR AMUX1     = 11;
const UCHAR AMUX2_1   = 12;
const UCHAR AMUX2_2   = 13;
const UCHAR NUM_MUXES = 14;
const UCHAR NO_MUX    = 15;

const GalileoPinsClass::PORT_ATTRIBUTES g_Gen2PinAttributes[] =
{
    //gpioType           pullupExp   triStExp    muxA             Muxes (A,B) by function:      triStIn   Function_mask
    //             portBit     pullupBit   triStBit      muxB     Dio  Pwm  AnIn I2C  Spi  Ser     _pad
    { GPIO_FABRIC, 3,    EXP1, P0_1, EXP1, P0_0, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SER },            // D0
    { GPIO_FABRIC, 4,    EXP0, P1_5, EXP0, P1_4, MUX7,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 1, 0, FUNC_DIO | FUNC_SER },            // D1
    { GPIO_FABRIC, 5,    EXP1, P0_3, EXP1, P0_2, MUX10,  NO_MUX,  1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SER },            // D2
    { GPIO_FABRIC, 6,    EXP0, P0_1, EXP0, P0_0, MUX0,   MUX9,    0,0, 1,0, 0,0, 0,0, 0,0, 0,1, 1, 0, FUNC_DIO | FUNC_PWM | FUNC_SER }, // D3
    { GPIO_LEGRES, 4,    EXP1, P0_5, EXP1, P0_4, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO },                       // D4
    { GPIO_LEGCOR, 0,    EXP0, P0_3, EXP0, P0_2, MUX1,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D5
    { GPIO_LEGCOR, 1,    EXP0, P0_5, EXP0, P0_4, MUX2,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D6
    { GPIO_EXP1,   P0_6, EXP1, P0_7, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D7
    { GPIO_EXP1,   P1_0, EXP1, P1_1, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D8
    { GPIO_LEGRES, 2,    EXP0, P0_7, EXP0, P0_6, MUX3,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D9
    { GPIO_FABRIC, 2,    EXP0, P1_3, EXP0, P1_2, MUX6,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D10
    { GPIO_LEGRES, 3,    EXP0, P1_1, EXP0, P1_0, MUX4,   MUX5,    0,0, 1,0, 0,0, 0,0, 0,1, 0,0, 1, 0, FUNC_DIO | FUNC_PWM | FUNC_SPI }, // D11
    { GPIO_FABRIC, 7,    EXP1, P1_3, EXP1, P1_2, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SPI },            // D12
    { GPIO_LEGRES, 5,    EXP0, P1_7, EXP0, P1_6, MUX8,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 1, 0, FUNC_DIO | FUNC_SPI },            // D13
    { GPIO_EXP2,   P0_0, EXP2, P0_1, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A0
    { GPIO_EXP2,   P0_2, EXP2, P0_3, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A1
    { GPIO_EXP2,   P0_4, EXP2, P0_5, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A2
    { GPIO_EXP2,   P0_6, EXP2, P0_7, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A3
    { GPIO_EXP2,   P1_0, EXP2, P1_1, NO_X, 0,    AMUX1,  AMUX2_1, 1,1, 0,0, 1,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }, // A4
    { GPIO_EXP2,   P1_2, EXP2, P1_3, NO_X, 0,    AMUX1,  AMUX2_2, 1,1, 0,0, 1,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }  // A5
};

// The array of MUX Attributes by MUX number.
const GalileoPinsClass::MUX_ATTRIBUTES g_Gen2MuxAttributes[NUM_MUXES] =
{
    { PWM,  LED1 },     // MUX0
    { PWM,  LED2 },     // MUX1
    { PWM,  LED5 },     // MUX2
    { PWM,  LED7 },     // MUX3
    { PWM,  LED9 },     // MUX4
    { EXP1, P1_4 },     // MUX5
    { PWM,  LED11 },    // MUX6
    { EXP1, P1_5 },     // MUX7
    { EXP1, P1_6 },     // MUX8
    { PWM,  LED12 },    // MUX9
    { PWM,  LED13 },    // MUX10
    { EXP2, P1_4 },     // AMUX1
    { PWM,  LED14 },    // AMUX2_1
    { PWM,  LED15 }     // AMUX2_2
};

const GalileoPinsClass::EXP_ATTRIBUTES g_Gen2ExpAttributes[] = 
{
    { PCAL9535A, 0x25 },    // EXP0
    { PCAL9535A, 0x26 },    // EXP1
    { PCAL9535A, 0x27 },    // EXP2
    { PCA9685,   0x47 }     // PWM
};

GalileoPinsClass::GalileoPinsClass()
    :
    m_PinAttributes(g_Gen2PinAttributes),
    m_MuxAttributes(g_Gen2MuxAttributes),
    m_ExpAttributes(g_Gen2ExpAttributes)
{
}

// Function to make sure a pin number is in range.
BOOL GalileoPinsClass::_pinNumberIsValid(UCHAR pin)
{
    return (pin < NUM_ARDUINO_PINS);
}


//
// Routine to set the current function of a multi-function pin.
// INPUT:
//      pin - The number of the pin in question.
//      function - The function to be used on the pin.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinFunction(UCHAR pin, UCHAR function)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    // Verify the pin number is in range.
    status = _pinNumberIsValid(pin);
    if (!status) { error = ERROR_INVALID_PARAMETER; }

    // Verify the requsted function is supported on this pin.
    if (status && ((m_PinAttributes[pin].funcMask & function) == 0))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        if (function == FUNC_DIO)
        {
            status = _setPinDigitalIo(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_PWM)
        {
            status = _setPinPwm(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_AIN)
        {
            status = _setPinAnalogInput(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_I2C)
        {
            status = _setPinI2c(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_SPI)
        {
            status = _setPinSpi(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_SER)
        {
            status = _setPinHwSerial(pin);
            if (!status) { error = GetLastError(); }
        }
        else
        {
            status = FALSE;
            ERROR_INVALID_PARAMETER;
        }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to Digital I/O.
// This routine assumes the caller has verified pin number range
// and that Digital I/O is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinDigitalIo(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at least one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Digital I/O.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].digIoMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Digital I/O.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].digIoMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to PWM.
// This routine assumes the caller has verified pin number range
// and that PWM is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinPwm(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for PWM.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].pwmMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for PWM.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].pwmMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to Analog Input.
// This routine assumes the caller has verified pin number range
// and that Analog Input is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinAnalogInput(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Analog Input.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].anInMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Analog Input.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].anInMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to I2C.
// This routine assumes the caller has verified pin number range
// and that I2C bus use is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinI2c(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for I2C bus use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].i2cMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for I2C bus use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].i2cMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to SPI.
// This routine assumes the caller has verified pin number range
// and that SPI bus use is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinSpi(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for SPI bus use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].spiMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for SPI bus use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].spiMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the current function of a pin to Hardware Serial.
// This routine assumes the caller has verified pin number range
// and that Hardware Serial is supported on the specified pin.
// INPUT:
//      pin - The number of the pin in question.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinHwSerial(UCHAR pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Hardware Serial use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].serMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Hardware Serial use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].serMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

//
// Routine to set the mode and drive type of a pin (Input, Output, etc.)
// INPUT:
//      pin - The number of the pin in question.
//      mode - The desired mode: INPUT or OUTPUT
//      pullup: true - enable pin pullup resistor, false - disable pullup
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setPinMode(UCHAR pin, UCHAR mode, bool pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    if (!_pinNumberIsValid(pin))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }
    else if ((mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        // Set the pin direction on the device that supports this pin.
        switch (m_PinAttributes[pin].gpioType)
        {
        case GPIO_FABRIC:
            status = g_fabricGpio.setPinDirection(m_PinAttributes[pin].portBit, mode);
            break;
        case GPIO_LEGRES:
            // TODO: Code here...
            break;
        case GPIO_LEGCOR:
            // TODO: Code here...
            break;
        case GPIO_EXP1:
        case GPIO_EXP2:
            // TODO: Code here...
            break;
        default:
            status = FALSE;
            error = DNS_ERROR_INVALID_TYPE;
        }
    }

    if (status)
    {
        // Configure the pin drivers as needed.
        status = _configurePinDrivers(pin, mode);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Configure the pin pullup as requested.
        status = _configurePinPullup(pin, pullup);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to set an I/O Expander bit to the specified direction.
BOOL GalileoPinsClass::_setExpBitDirection(UCHAR expNo, UCHAR bitNo, UCHAR direction)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR i2cAdr = 0;               // I2C address of I/O Expander


    // Get the I2C Address of the I/O Expander in question.
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    //
    // Determine what type of I/O Expander we are dealing with.
    //

    if (m_ExpAttributes[expNo].Exp_Type == PCAL9535A)
    {
        // Set the bit of the I/O Expander chip to the desired direction.
        status = PCAL9535ADevice::SetBitDirection(i2cAdr, bitNo, direction);
        if (!status) { error = GetLastError(); }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == PCA9685)
    {
        // Set the bit of the PWM chip to the desired direction.
        status = PCA9685Device::SetBitDirection(i2cAdr, bitNo, direction);
        if (!status) { error = GetLastError(); }
    }
    else
    {
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to set an I/O Expander bit to the specified state.
BOOL GalileoPinsClass::_setExpBitToState(UCHAR expNo, UCHAR bitNo, UCHAR state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR i2cAdr = 0;               // I2C address of I/O Expander


    // Get the I2C Address of the I/O Expander in question.
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    //
    // Determine what type of I/O Expander we are dealing with.
    //

    if (m_ExpAttributes[expNo].Exp_Type == PCAL9535A)
    {
        // Set the bit of the I/O Expander chip to the desired state.
        status = PCAL9535ADevice::SetBitState(i2cAdr, bitNo, state);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Set the bit of the I/O Expander chip to be an output.
            status = PCAL9535ADevice::SetBitDirection(i2cAdr, bitNo, DIRECTION_OUT);
            if (!status) { error = GetLastError(); }
        }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == PCA9685)
    {
        // Set the bit of the PWM chip to the desired state.
        status = PCA9685Device::SetBitState(i2cAdr, bitNo, state);
        if (!status) { error = GetLastError(); }
    }
    else
    {
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to configure the pin drivers to reflect the direction of this pin.
// This method assumes pin and mode have been verified to be in range.
BOOL GalileoPinsClass::_configurePinDrivers(UCHAR pin, UCHAR mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR expNo = 0;                // I/O Expander number
    UCHAR bitNo = 0;                // Bit number on I/O Expander
    UCHAR state = 0;                // Desired pin state


    if ((mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        // Determine which I/O Expander pin controls the GPIO-pin drivers.
        expNo = m_PinAttributes[pin].triStExp;
        bitNo = m_PinAttributes[pin].triStBit;

        // Determine the Exp pin state to select the needed driver.
        state = m_PinAttributes[pin].triStIn;
        if (mode == DIRECTION_OUT)
        {
            state = state ^ 0x01;
        }

        // Set the I/O Expander bit to the correct state.
        status = _setExpBitToState(expNo, bitNo, state);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

// Method to configure the pin pullup as specified.
// This method assumes pin has been verified to be in the valid range.
BOOL GalileoPinsClass::_configurePinPullup(UCHAR pin, bool pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR expNo = 0;                // I/O Expander number
    UCHAR bitNo = 0;                // Bit number on I/O Expander
    UCHAR state = 0;                // Desired pin state

    // Determine which I/O Expander pin controls the GPIO-pin drivers.
    expNo = m_PinAttributes[pin].pullupExp;
    bitNo = m_PinAttributes[pin].pullupBit;

    // Set the I/O Expander bit high.
    status = _setExpBitToState(expNo, bitNo, 1);
    if (!status)  { error = GetLastError(); }

    // If the pullup is wanted:
    if (pullup)
    {
        // Make the I/O Expander bit an output.
        status = _setExpBitDirection(expNo, bitNo, DIRECTION_OUT);
        if (!status)  { error = GetLastError(); }
    }

    // If no pullup is wanted:
    else
    {
        // Make the I/O Expander bit as an input.
        status = _setExpBitDirection(expNo, bitNo, DIRECTION_IN);
        if (!status)  { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

//
// Routine to set a MUX to select a deired signal.
// INPUT:
//      mux - The number of the MUX in question.
//      selection - The desired state of the MUX "select" input.
// RETURN:
//      TRUE - Success.
//      FALSE - Failure.  GetLastError() provides error code.
//
BOOL GalileoPinsClass::_setMux(UCHAR mux, UCHAR selection)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR expNo = 0;                // I/O Expander number
    UCHAR bitNo = 0;                // Bit number on I/O Expander


    // If the MUX number is outside the valid range, fail.
    if (mux >= NUM_MUXES)
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Determine which I/O Expander and pin drive the MUX select input.
        expNo = m_MuxAttributes[mux].selectExp;
        bitNo = m_MuxAttributes[mux].selectBit;

        // Set the I/O Expander bit to the correct state.
        status = _setExpBitToState(expNo, bitNo, selection);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

BOOL GalileoPinsClass::_setPinState(UCHAR pin, UCHAR state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (!_pinNumberIsValid(pin))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (state > 1)
    {
        status = FALSE;
        error = ERROR_INVALID_STATE;
    }

    if (status)
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
        case GPIO_FABRIC:
            return g_fabricGpio.setPinState(m_PinAttributes[pin].portBit, state);
        case GPIO_LEGRES:
            // TODO: Code here and remove the break if appropriate.
            break;
        case GPIO_LEGCOR:
            // TODO: Code here and remove the break if appropriate.
            break;
        case GPIO_EXP1:
            return PCAL9535ADevice::SetBitState(
                m_ExpAttributes[GPIO_EXP1].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        case GPIO_EXP2:
            return PCAL9535ADevice::SetBitState(
                m_ExpAttributes[GPIO_EXP2].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        default:
            status = FALSE;
            error = DNS_ERROR_INVALID_TYPE;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}





