// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "CY8C9540ASupport.h"
#include "ExpanderDefs.h"
#include "I2cController.h"


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
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
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
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status && (state != HIGH) && (state != LOW))
    {
        status = FALSE;
        error = ERROR_INVALID_STATE;
    }

    // Calculate the address of the output register for the port in question.
    if (status)
    {
        portRegAdr[0] = (UCHAR) (OUT_BASE_ADR + port);
    }

    //
    // Read what has been sent to the output port.
    //

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Send the address of the output port to the I/O Expander chip.
        status = transaction.queueWrite(portRegAdr, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Read the port contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Register a callback to set the desired bit state.
        status = transaction.queueCallback([&dataBuf, bit, state]()
        {
            dataBuf[0] = dataBuf[0] & (~(1 << bit));
            dataBuf[0] = dataBuf[0] | ((state & 0x01) << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Send the address of the output port to the I/O Expander chip.
        status = transaction.queueWrite(portRegAdr, 1, TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Send the desired state of the port bit to the I/O Expander chip.
        status = transaction.queueWrite(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Actually perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to read.
\param[out] state The state of the port bit: HIGH or LOW.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
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
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    // Calculate the address of the input register for the port in question.
    if (status)
    {
        portRegAdr[0] = (UCHAR)(IN_BASE_ADR + port);
    }

    //
    // Read the input port.
    //

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Send the address of the input port to the I/O Expander chip.
        status = transaction.queueWrite(portRegAdr, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Read the port contents from the I/O Expander chip.
        status = transaction.queueRead(dataBuf, sizeof(dataBuf));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Extract the bit we want from the input port data.
        state = (dataBuf[0] >> bit) & 0x01;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method sets the direction and drive characterics of a port bit of the I/O Expander.
\param[in] i2cAdr The I2C address of the chip.
\param[in] portBit The number of the port bit to modify.
\param[in] direction The direction to configure for the pin (DIRECTION_IN or DIRECTION_OUT).
\param[in] pullup TRUE - enable the pullup resistor on the pin, FALSE - disable pullup.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::SetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG direction, BOOL pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
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
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    //
    // Select the port the pin configuration registers refer to.
    //

    if (status)
    {
        // Queue a write of the port select register address.
        status = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR) port;
        status = transaction.queueWrite(portSelectData, sizeof(portSelectData));
        if (!status) { error = GetLastError(); }
    }

    //
    // Set the desired direction for the bit in question.
    //

    if (status)
    {
        // Queue a write of the port direction register address.
        status = transaction.queueWrite(directionAdr, sizeof(directionAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a read of the direction register.
        status = transaction.queueRead(directionData, sizeof(directionData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Determine the desired state of the pin direction bit.
        state = 1;
        if ((direction == DIRECTION_OUT) || pullup)
        {
            state = 0;
        }

        // Register a callback to set the desired direction for the port bit in question.
        status = transaction.queueCallback([&directionData, bit, state]()
        {
            directionData[0] = directionData[0] & (~(1 << bit));
            directionData[0] = directionData[0] | ((state & 0x01) << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Queue a write of the address of the Direction register to the I/O Expander chip.
        status = transaction.queueWrite(directionAdr, sizeof(directionAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write to send the modified value back to the direction register.
        status = transaction.queueWrite(directionData, sizeof(directionData));
        if (!status) { error = GetLastError(); }
    }

    //
    // Select the drive type.
    //
    
    if (status)
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
        status = transaction.queueWrite(driveAdr, sizeof(driveAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a read of the drive register.
        status = transaction.queueRead(driveData, sizeof(driveData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Register a callback to set the appropriate bit in the drive register data.
        status = transaction.queueCallback([&driveData, bit]()
        {
            driveData[0] = driveData[0] | (0x01U << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Queue a write of pin drive register address.
        status = transaction.queueWrite(driveAdr, sizeof(driveAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write to send the modified data back to the port drive register.
        status = transaction.queueWrite(driveData, sizeof(driveData));
        if (!status) { error = GetLastError(); }
    }

    //
    // If the pin is an input with a pull-up, set the pin on the output port high.
    //

    if (status && (direction != DIRECTION_OUT) && pullup)
    {
        // Queue a write of the port output register address.
        outAdr[0] = (UCHAR)(OUT_BASE_ADR + port);
        status = transaction.queueWrite(outAdr, sizeof(outAdr), TRUE);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Queue a read of the port output register.
            status = transaction.queueRead(outData, sizeof(outData));
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Register a callback to set the appropriate bit in the drive register data.
            status = transaction.queueCallback([&outData, bit]()
            {
                outData[0] = outData[0] | (0x01U << bit);
                return TRUE;
            }
            );
        }

        if (status)
        {
            // Queue a write of port output register address.
            status = transaction.queueWrite(outAdr, sizeof(outAdr), TRUE);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue a write to send the modified data back to the port drive register.
            status = transaction.queueWrite(outData, sizeof(outData));
            if (!status) { error = GetLastError(); }
        }
    }

    //
    //  Perform all the I2C transfers specified above.
    //
    if (status)
    {
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

// This is a placeholder in case this function is needed in the future.
BOOL CY8C9540ADevice::GetBitDirection(ULONG i2cAdr, ULONG portBit, ULONG & direction)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (!status) { SetLastError(error); }
    return status;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to configure as a PWM output.
\param[in] pwmChan The PWM channel associated with the port bit.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::SetPortbitPwm(ULONG i2cAdr, ULONG portBit, ULONG pwmChan)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
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
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the expander bit to be an output with strong drive.
        status = SetBitDirection(i2cAdr, portBit, DIRECTION_OUT, FALSE);
        if (!status) { error = GetLastError(); }
    }

    // Select PWM to output on the pin.
    if (status)
    {
        // Queue a write of the port select register address.
        status = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR)port;
        status = transaction.queueWrite(portSelectData, sizeof(portSelectData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the Select PWM register address.
        status = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a read of the Select PWM register.
        status = transaction.queueRead(selectPwmData, sizeof(selectPwmData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Register a callback to set the Select PWM bit for the correct port bit.
        status = transaction.queueCallback([&selectPwmData, bit]()
        {
            selectPwmData[0] = selectPwmData[0] | (0x01 << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Queue a write of the Select PWM register address.
        status = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write to send the modified value back to the Select PWM register.
        status = transaction.queueWrite(selectPwmData, sizeof(selectPwmData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        //  Perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the expander port bit high.
        status = SetBitState(i2cAdr, portBit, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the PWM frequency.
        status = _configurePwmChannelFrequency(i2cAdr, pwmChan);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to configure for Digital I/O.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::SetPortbitDio(ULONG i2cAdr, ULONG portBit)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
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
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    // De-select PWM for the pin.
    if (status)
    {
        // Queue a write of the port select register address.
        status = transaction.queueWrite(portSelectAdr, sizeof(portSelectAdr));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the port select data.
        portSelectData[0] = (UCHAR)port;
        status = transaction.queueWrite(portSelectData, sizeof(portSelectData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the Select PWM register address.
        status = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a read of the Select PWM register.
        status = transaction.queueRead(selectPwmData, sizeof(selectPwmData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Register a callback to clear the Select PWM bit for the correct port bit.
        status = transaction.queueCallback([&selectPwmData, bit]()
        {
            selectPwmData[0] = selectPwmData[0] & ~(0x01 << bit);
            return TRUE;
        }
        );
    }

    if (status)
    {
        // Queue a write of the Select PWM register address.
        status = transaction.queueWrite(selectPwmAdr, sizeof(selectPwmAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write to send the modified value back to the Select PWM register.
        status = transaction.queueWrite(selectPwmData, sizeof(selectPwmData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        //  Perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Set the width of the positive pulses on one of the PWM channels.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] channel The channel on the PWM chip for which to set the pulse width.
\param[in] dutyCycle The desired duty-cycle of the positive pulses (0-0xFFFFFFFF for 0-100%).
\return TRUE success.FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::SetPwmDutyCycle(ULONG i2cAdr, ULONG channel, ULONG dutyCycle)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    ULONGLONG pulseWidth = 0;
    UCHAR chanSelectAdr[1] = { PWM_SELECT_ADR };    // Address of PWM Channel Select register
    UCHAR chanSelectData[1] = { 0 };                // Data buffer for writing to Channel Select register
    UCHAR pulseWidthAdr[1] = { PULSE_WIDTH_ADR };   // Address of PWM Pulsewidth register
    UCHAR pulseWidthData[1] = { 0 };                // Data buffer for writing Select PWM register


    if (channel >= PWM_CHAN_COUNT)
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Calculate the pulse width value for the specified dutyCycle.
        pulseWidth = ((dutyCycle * ((ULONGLONG)((1<<PWM_BITS)-1))) + 0x7FFFFFFFULL) / 0xFFFFFFFFULL;
        pulseWidthData[0] = (UCHAR)pulseWidth;
    }

    if (status)
    {
        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the PWM Select register address.
        status = transaction.queueWrite(chanSelectAdr, sizeof(chanSelectAdr));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the PWM channel number data.
        chanSelectData[0] = (UCHAR)channel;
        status = transaction.queueWrite(chanSelectData, sizeof(chanSelectData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the PWM Period register address.
        status = transaction.queueWrite(pulseWidthAdr, sizeof(pulseWidthAdr), TRUE);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the PWM Period data.
        status = transaction.queueWrite(pulseWidthData, sizeof(pulseWidthData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        //  Perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method gets the desired frequency from the array of frequencies by channel
number.  The only frequency currently supported i 367 hz.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] channel The channel on the PWM chip for which to set the frequencey.
\return TRUE success.FALSE failure, GetLastError() provides error code.
*/
BOOL CY8C9540ADevice::_configurePwmChannelFrequency(ULONG i2cAdr, ULONG channel)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    ULONGLONG pulseWidth = 0;
    UCHAR pwmRegBaseAdr[1] = { PWM_SELECT_ADR };    // Address of PWM Channel Select register
    UCHAR pwmRegData[3] = { 0, PWM_CLK_94K, 0xFF }; // Data buffer for writing to PWM registers


    if (channel >= PWM_CHAN_COUNT)
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Put channel number in PWM reg data so it will land in the PWM Select register.
        pwmRegData[0] = (UCHAR)channel;

        // Set the I2C address of the I/O Expander we want to talk to.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue a write of the PWM register data.
        status = transaction.queueWrite(pwmRegData, sizeof(pwmRegData));
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        //  Perform the I2C transfer specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}



