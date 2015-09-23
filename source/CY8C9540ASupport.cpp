// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "CY8C9540ASupport.h"
#include "ExpanderDefs.h"
#include "I2c.h"
#include "ErrorCodes.h"
#include "ArduinoCommon.h"

const ULONG CY8C9540ADevice::PWM_BITS          =    8;  // This PWM chip has 8 bits of resolution.
const ULONG CY8C9540ADevice::PWM_CHAN_COUNT    =    8;  // This chip has 8 PWM channels.
const ULONG CY8C9540ADevice::PORT_COUNT        =    6;  // Number of ports supported by this chip.

const ULONG CY8C9540ADevice::IN_BASE_ADR       = 0x00;  // Base address of Input Port registers.
const ULONG CY8C9540ADevice::OUT_BASE_ADR      = 0x08;  // Base address of Output Port registers.
const ULONG CY8C9540ADevice::INT_STAT_BASE_ADR = 0x10;  // Base address of Inerrupt Status registers.
const ULONG CY8C9540ADevice::PORT_SELECT_ADR   = 0x18;  // Address of Port Select register.
const ULONG CY8C9540ADevice::INT_MASK_ADR      = 0x19;  // Address of Interrupt Mask register.
const ULONG CY8C9540ADevice::SEL_PWM_ADR       = 0x1A;  // Address of Select PWM for Port Output register.
const ULONG CY8C9540ADevice::INVERSION_ADR     = 0x1B;  // Address of Inversion register.
const ULONG CY8C9540ADevice::PIN_DIR_ADR       = 0x1C;  // Address of Pin Direction register (0: Out, 1: In)
const ULONG CY8C9540ADevice::PULL_UP_ADR       = 0x1D;  // Address of Drive Mode - Pull Up register.
const ULONG CY8C9540ADevice::PULL_DOWN_ADR     = 0x1E;  // Address of Drive Mode - Pull Down register.
const ULONG CY8C9540ADevice::OPEN_HIGH_ADR     = 0x1F;  // Address of Drive Mode - Open Drain High register.
const ULONG CY8C9540ADevice::OPEN_LOW_ADR      = 0x20;  // Address of Drive Mode - Open Drain Low register.
const ULONG CY8C9540ADevice::DRIVE_STRONG_ADR  = 0x21;  // Address of Drive Mode - Strong register.
const ULONG CY8C9540ADevice::SLOW_STRONG_ADR   = 0x22;  // Address of Drive Mode - Slow Strong register.
const ULONG CY8C9540ADevice::HIGH_Z_ADR        = 0x23;  // Address of Drive Mode - High-Z register.
const ULONG CY8C9540ADevice::PWM_SELECT_ADR    = 0x28;  // Address of PWM Select register.
const ULONG CY8C9540ADevice::CONFIG_PWM_ADR    = 0x29;  // Address of Config PWM register.
const ULONG CY8C9540ADevice::PERIOD_PWM_ADR    = 0x2A;  // Address of Period PWM register.
const ULONG CY8C9540ADevice::PULSE_WIDTH_ADR   = 0x2B;  // Address of Pulse Width PWM register.
const ULONG CY8C9540ADevice::DIVIDER_ADR       = 0x2C;  // Address of Programmable Diver register.
const ULONG CY8C9540ADevice::ENABLE_EEE_ADR    = 0x2D;  // Address of Enable WDE, EEE, EERO register.
const ULONG CY8C9540ADevice::ID_STATUS_ADR     = 0x2E;  // Address of Device ID/Status register.
const ULONG CY8C9540ADevice::WATCHDOG_ADR      = 0x2F;  // Address of Watchdog register.
const ULONG CY8C9540ADevice::COMMAND_ADR       = 0x30;  // Address of Command register.

const ULONG CY8C9540ADevice::PWM_CLK_32K       = 0x00;  // Config PWM register value for 32 kHz clock.
const ULONG CY8C9540ADevice::PWM_CLK_24M       = 0x01;  // Config PWM register value for 24 MHz clock.
const ULONG CY8C9540ADevice::PWM_CLK_1M5       = 0x02;  // Config PWM register value for 1.5 MHz clock.
const ULONG CY8C9540ADevice::PWM_CLK_94K       = 0x03;  // Config PWM register value for 93.75 kHz clock.
const ULONG CY8C9540ADevice::PWM_CLK_368       = 0x04;  // Config PWM register value for 367.6 Hz clock.
const ULONG CY8C9540ADevice::PWM_CLK_PREV      = 0x05;  // Config PWM register value for Previous PWM clock.

const ULONG CY8C9540ADevice::CMD_STORE_CONFIG  =    1;  // Command to store device config in EEPROM.
const ULONG CY8C9540ADevice::CMD_FACTORY_DFLTS =    2;  // Command to restore factory defaults.
const ULONG CY8C9540ADevice::CMD_WR_EE_DFLTS   =    3;  // Command to write EEPROM POR defaults.
const ULONG CY8C9540ADevice::CMD_RD_EE_DFLTS   =    4;  // Command to read EEPROM POR defaults.
const ULONG CY8C9540ADevice::CMD_WR_CONFIG     =    5;  // Command to write device configuration.
const ULONG CY8C9540ADevice::CMD_RD_CONFIG     =    6;  // Command to read device configuation.
const ULONG CY8C9540ADevice::CMD_LD_DEFAULTS   =    7;  // Command to reconfigure with stored POR defaults.

/**
This method takes the actions needed to set a port bit of the I/O Expander chip to the desired state.
\param[in] i2cAdr The I2C address of the chip.
\param[in] portBit The number of the port bit to modify.
\param[in] state The state to set the port bit to: HIGH or LOW.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG port;                             // Number of the port we are modifying
    ULONG bit;                              // Number of the bit to modify within the port
    UCHAR portRegAdr[1] = { 0 };            // Buffer for port output register address
    UCHAR dataBuf[1] = { 0 };               // Buffer for port data

    // Calculate the number of the port and the bit we are modifying.
    port = portBit >> 3;
    bit = portBit & 0x07;

    if (port >= PORT_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr) && (state != HIGH) && (state != LOW))
    {
        hr = DMAP_E_INVALID_PIN_STATE_SPECIFIED;
    }

    // Calculate the address of the output register for the port in question.
    if (SUCCEEDED(hr))
    {
        portRegAdr[0] = (UCHAR) (OUT_BASE_ADR + port);
    }

    //
    // Read what has been sent to the output port.
    //

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Send the address of the output port to the I/O Expander chip.
        hr = transaction.queueWrite(portRegAdr, 1);
        
    }

    if (SUCCEEDED(hr))
    {
        // Read the port contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Register a callback to set the desired bit state.
        hr = transaction.queueCallback([&dataBuf, bit, state]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | ((state & 0x01) << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Send the address of the output port to the I/O Expander chip.
        hr = transaction.queueWrite(portRegAdr, 1, TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Send the desired state of the port bit to the I/O Expander chip.
        hr = transaction.queueWrite(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Actually perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to read.
\param[out] state The state of the port bit: HIGH or LOW.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG port;                             // Number of the port we are reading
    ULONG bit;                              // Number of the bit to read on the port
    UCHAR portRegAdr[1] = { 0 };            // Buffer for port input register address
    UCHAR dataBuf[1] = { 0 };               // Buffer for port data

    // Calculate the number of the port and the bit we are reading.
    port = portBit >> 3;
    bit = portBit & 0x07;

    if (port >= PORT_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    // Calculate the address of the input register for the port in question.
    if (SUCCEEDED(hr))
    {
        portRegAdr[0] = (UCHAR)(IN_BASE_ADR + port);
    }

    //
    // Read the input port.
    //

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Send the address of the input port to the I/O Expander chip.
        hr = transaction.queueWrite(portRegAdr, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Read the port contents from the I/O Expander chip.
        hr = transaction.queueRead(dataBuf, sizeof(dataBuf));
    }

    if (SUCCEEDED(hr))
    {
        // Perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }

    if (SUCCEEDED(hr))
    {
        // Extract the bit we want from the input port data.
        state = (dataBuf[0] >> bit) & 0x01;
    }
    
    return hr;
}

/**
This method sets the direction and drive characterics of a port bit of the I/O Expander.
\param[in] i2cAdr The I2C address of the chip.
\param[in] portBit The number of the port bit to modify.
\param[in] direction The direction to configure for the pin (DIRECTION_IN or DIRECTION_OUT).
\param[in] pullup TRUE - enable the pullup resistor on the pin, FALSE - disable pullup.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::SetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG direction, BOOL pullup)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG port;                                     // Number of the port we are modifying
    ULONG bit;                                      // Number of the bit to modify within the port
    UCHAR portSelectAdr[1] = { PORT_SELECT_ADR };   // Address of Port Select register
    UCHAR portSelectData[1] = { 0 };                // Data buffer for writing to Port Select register
    UCHAR directionAdr[1] = { PIN_DIR_ADR };        // Address of Pin Direction register
    UCHAR directionData[1] = { 0 };                 // Data buffer for read/modify/write of Pin Direction
    UCHAR driveAdr[1] = { 0 };                      // Pin drive register address
    UCHAR driveData[1] = { 0 };                     // Data buffer for pin drive read/modify/write operation
    UCHAR outAdr[1] = { 0 };                        // Pin output state register address
    UCHAR outData[1] = { 0 };                       // Data buffer for pin state read/modify/write operation
    UCHAR state;                                    // Variable for bit state


    // Calculate the number of the port and the bit we are modifying.
    port = portBit >> 3;
    bit = portBit & 0x07;

    if (port >= PORT_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    //
    // Select the port the pin configuration registers refer to.
    //

    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select register address.
        hr = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR) port;
        hr = transaction.queueWrite(portSelectData, sizeof(portSelectData));
    }

    //
    // Set the desired direction for the bit in question.
    //

    if (SUCCEEDED(hr))
    {
        // Queue a write of the port direction register address.
        hr = transaction.queueWrite(directionAdr, sizeof(directionAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a read of the direction register.
        hr = transaction.queueRead(directionData, sizeof(directionData));
    }

    if (SUCCEEDED(hr))
    {
        // Determine the desired state of the pin direction bit.
        state = 1;
        if ((direction == DIRECTION_OUT) || pullup)
        {
            state = 0;
        }

        // Register a callback to set the desired direction for the port bit in question.
        hr = transaction.queueCallback([&directionData, bit, state]()
        {
            directionData[0] = directionData[0] & (~(1 << bit));
            directionData[0] = directionData[0] | ((state & 0x01) << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the address of the Direction register to the I/O Expander chip.
        hr = transaction.queueWrite(directionAdr, sizeof(directionAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write to send the modified value back to the direction register.
        hr = transaction.queueWrite(directionData, sizeof(directionData));
    }

    //
    // Select the drive type.
    //
    
    if (SUCCEEDED(hr))
    {
        // Determine what drive type is needed.
        if (direction == DIRECTION_OUT)
        {
            driveAdr[0] = DRIVE_STRONG_ADR;
        }
        else if (pullup)
        {
            driveAdr[0] = PULL_UP_ADR;
        }
        else
        {
            driveAdr[0] = HIGH_Z_ADR;
        }

        // Queue a write of the pin drive register address.
        hr = transaction.queueWrite(driveAdr, sizeof(driveAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a read of the drive register.
        hr = transaction.queueRead(driveData, sizeof(driveData));
    }

    if (SUCCEEDED(hr))
    {
        // Register a callback to set the appropriate bit in the drive register data.
        hr = transaction.queueCallback([&driveData, bit]()
        {
            driveData[0] = driveData[0] | (0x01U << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of pin drive register address.
        hr = transaction.queueWrite(driveAdr, sizeof(driveAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write to send the modified data back to the port drive register.
        hr = transaction.queueWrite(driveData, sizeof(driveData));
    }

    //
    // If the pin is an input with a pull-up, set the pin on the output port high.
    //

    if (SUCCEEDED(hr) && (direction != DIRECTION_OUT) && pullup)
    {
        // Queue a write of the port output register address.
        outAdr[0] = (UCHAR)(OUT_BASE_ADR + port);
        hr = transaction.queueWrite(outAdr, sizeof(outAdr), TRUE);

        if (SUCCEEDED(hr))
        {
            // Queue a read of the port output register.
            hr = transaction.queueRead(outData, sizeof(outData));
        }

        if (SUCCEEDED(hr))
        {
            // Register a callback to set the appropriate bit in the drive register data.
            hr = transaction.queueCallback([&outData, bit]()
            {
                outData[0] = outData[0] | (0x01U << bit);
                return S_OK;
            }
            );
        }

        if (SUCCEEDED(hr))
        {
            // Queue a write of port output register address.
            hr = transaction.queueWrite(outAdr, sizeof(outAdr), TRUE);
        }

        if (SUCCEEDED(hr))
        {
            // Queue a write to send the modified data back to the port drive register.
            hr = transaction.queueWrite(outData, sizeof(outData));
        }
    }

    //
    //  Perform all the I2C transfers specified above.
    //
    if (SUCCEEDED(hr))
    {
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

// This is a placeholder in case this function is needed in the future.
HRESULT CY8C9540ADevice::GetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG & direction)
{
    HRESULT hr = S_OK;
    
    return hr;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to configure as a PWM output.
\param[in] pwmChan The PWM channel associated with the port bit.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::SetPortbitPwm(ULONG i2cAdr, ULONG portBit, ULONG pwmChan)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG port;                                     // Number of the port we are modifying
    ULONG bit;                                      // Number of the bit to modify within the port
    UCHAR portSelectAdr[1] = { PORT_SELECT_ADR };   // Address of Port Select register
    UCHAR portSelectData[1] = { 0 };                // Data buffer for writing to Port Select register
    UCHAR selectPwmAdr[1] = { SEL_PWM_ADR };        // Address of Select PWM register
    UCHAR selectPwmData[1] = { 0 };                 // Data buffer for writing Select PWM register

    // Calculate the number of the port and the bit we are dealing with.
    port = portBit >> 3;
    bit = portBit & 0x07;

    if (port >= PORT_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Set the expander bit to be an output with strong drive.
        hr = SetBitDirection(i2cAdr, portBit, DIRECTION_OUT, FALSE);
    }

    // Select PWM to output on the pin.
    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select register address.
        hr = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR)port;
        hr = transaction.queueWrite(portSelectData, sizeof(portSelectData));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the Select PWM register address.
        hr = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a read of the Select PWM register.
        hr = transaction.queueRead(selectPwmData, sizeof(selectPwmData));
    }

    if (SUCCEEDED(hr))
    {
        // Register a callback to set the Select PWM bit for the correct port bit.
        hr = transaction.queueCallback([&selectPwmData, bit]()
        {
            selectPwmData[0] = selectPwmData[0] | (0x01 << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the Select PWM register address.
        hr = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write to send the modified value back to the Select PWM register.
        hr = transaction.queueWrite(selectPwmData, sizeof(selectPwmData));
    }

    if (SUCCEEDED(hr))
    {
        //  Perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }

    if (SUCCEEDED(hr))
    {
        // Set the expander port bit high.
        hr = SetBitState(i2cAdr, portBit, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Set the PWM frequency.
        hr = _configurePwmChannelFrequency(i2cAdr, pwmChan);
    }
    
    return hr;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to configure for Digital I/O.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::SetPortbitDio(ULONG i2cAdr, ULONG portBit)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG port;                                     // Number of the port we are modifying
    ULONG bit;                                      // Number of the bit to modify within the port
    UCHAR portSelectAdr[1] = { PORT_SELECT_ADR };   // Address of Port Select register
    UCHAR portSelectData[1] = { 0 };                // Data buffer for writing to Port Select register
    UCHAR selectPwmAdr[1] = { SEL_PWM_ADR };        // Address of Select PWM register
    UCHAR selectPwmData[1] = { 0 };                 // Data buffer for writing Select PWM register

    // Calculate the number of the port and the bit we are dealing with.
    port = portBit >> 3;
    bit = portBit & 0x07;

    if (port >= PORT_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    // De-select PWM for the pin.
    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select register address.
        hr = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR)port;
        hr = transaction.queueWrite(portSelectData, sizeof(portSelectData));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the Select PWM register address.
        hr = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a read of the Select PWM register.
        hr = transaction.queueRead(selectPwmData, sizeof(selectPwmData));
    }

    if (SUCCEEDED(hr))
    {
        // Register a callback to clear the Select PWM bit for the correct port bit.
        hr = transaction.queueCallback([&selectPwmData, bit]()
        {
            selectPwmData[0] = selectPwmData[0] & ~(0x01 << bit);
            return S_OK;
        }
        );
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the Select PWM register address.
        hr = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write to send the modified value back to the Select PWM register.
        hr = transaction.queueWrite(selectPwmData, sizeof(selectPwmData));
    }

    if (SUCCEEDED(hr))
    {
        //  Perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

/**
Set the width of the positive pulses on one of the PWM channels.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] channel The channel on the PWM chip for which to set the pulse width.
\param[in] dutyCycle The desired duty-cycle of the positive pulses (0-0xFFFFFFFF for 0-100%).
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::SetPwmDutyCycle(ULONG i2cAdr, ULONG channel, ULONG dutyCycle)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONGLONG pulseWidth = 0;
    UCHAR chanSelectAdr[1] = { PWM_SELECT_ADR };    // Address of PWM Channel Select register
    UCHAR chanSelectData[1] = { 0 };                // Data buffer for writing to Channel Select register
    UCHAR pulseWidthAdr[1] = { PULSE_WIDTH_ADR };   // Address of PWM Pulsewidth register
    UCHAR pulseWidthData[1] = { 0 };                // Data buffer for writing Select PWM register

    if (channel >= PWM_CHAN_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the pulse width value for the specified dutyCycle.
        pulseWidth = ((dutyCycle * ((ULONGLONG)((1<<PWM_BITS)-1))) + 0x7FFFFFFFULL) / 0xFFFFFFFFULL;
        pulseWidthData[0] = (UCHAR)pulseWidth;
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the PWM Select register address.
        hr = transaction.queueWrite(chanSelectAdr, sizeof(chanSelectAdr));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the PWM channel number data.
        chanSelectData[0] = (UCHAR)channel;
        hr = transaction.queueWrite(chanSelectData, sizeof(chanSelectData));
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the PWM Period register address.
        hr = transaction.queueWrite(pulseWidthAdr, sizeof(pulseWidthAdr), TRUE);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the PWM Period data.
        hr = transaction.queueWrite(pulseWidthData, sizeof(pulseWidthData));
    }

    if (SUCCEEDED(hr))
    {
        //  Perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

/**
This method gets the desired frequency from the array of frequencies by channel
number.  The only frequency currently supported i 367 hz.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] channel The channel on the PWM chip for which to set the frequencey.
\return HRESULT success or error code.
*/
HRESULT CY8C9540ADevice::_configurePwmChannelFrequency(ULONG i2cAdr, ULONG channel)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONGLONG pulseWidth = 0;
    UCHAR pwmRegBaseAdr[1] = { PWM_SELECT_ADR };    // Address of PWM Channel Select register
    UCHAR pwmRegData[3] = { 0, PWM_CLK_94K, 0xFF }; // Data buffer for writing to PWM registers

    if (channel >= PWM_CHAN_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Put channel number in PWM reg data so it will land in the PWM Select register.
        pwmRegData[0] = (UCHAR)channel;

        // Set the I2C address of the I/O Expander we want to talk to.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Queue a write of the PWM register data.
        hr = transaction.queueWrite(pwmRegData, sizeof(pwmRegData));
    }

    if (SUCCEEDED(hr))
    {
        //  Perform the I2C transfer specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}
