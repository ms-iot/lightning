// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "PCA9685Support.h"
#include "ExpanderDefs.h"
#include "I2cController.h"

// Start with "chip not initialized" status.
BOOL PCA9685Device::m_chipIsInitialized = FALSE;

// PWM prescale value for default pulse rate of 1000 pulses per second.
// Prescale = round(25000000/(4096 * pulse_rate)) - 1
UCHAR PCA9685Device::m_freqPreScale = 5;

const ULONG PCA9685Device::PWM_BITS         =   12;     // This PWM chip has 12 bits of resolution

const ULONG PCA9685Device::MODE1_ADR        = 0x00;     // Address of MODE1 register
const ULONG PCA9685Device::MODE2_ADR        = 0x01;     // Address of MODE2 register
const ULONG PCA9685Device::SUBADR1_ADR      = 0x02;     // Address of SUBADR1 register
const ULONG PCA9685Device::SUBADR2_ADR      = 0x03;     // Address of SUBADR1 register
const ULONG PCA9685Device::SUBADR3_ADR      = 0x04;     // Address of SUBADR1 register
const ULONG PCA9685Device::ALLCALLADR_ADR   = 0x05;     // Address of ALLCALLADR register
const ULONG PCA9685Device::LEDS_BASE_ADR    = 0x06;     // Base address of LED output registers
const ULONG PCA9685Device::REGS_PER_LED     = 0x04;     // Number of registers for each LED
const ULONG PCA9685Device::PRE_SCALE_ADR    = 0xFE;     // Address of frequency prescale register
const ULONG PCA9685Device::TestMode_ADR     = 0xFF;     // Address of TestMode register
const ULONG PCA9685Device::LED_COUNT        = 0x10;     // Number of "LED" ports on the chip
/**
This method takes the actions needed to set a port bit of the PWM chip to the desired state.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to modify.
\param[in] state The state to set the port bit to: HIGH or LOW.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL PCA9685Device::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR lowBitData[REGS_PER_LED] = { 0x00, 0x00, 0x00, 0x10 };    // Registers data to set bit low
    UCHAR highBitData[REGS_PER_LED] = { 0x00, 0x10, 0x00, 0x00 };   // Registers data to set bit high


    if (portBit >= LED_COUNT)
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status && (state != HIGH) && (state != LOW))
    {
        status = FALSE;
        error = ERROR_INVALID_STATE;
    }

    if (status)
    {
        // Make sure the PWM chip is initialized.
        status = _InitializeChip(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the I2C address of the PWM chip.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (portBit * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        status = transaction.queueWrite(&bitRegsAdr, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue sending the registers contents to set the specified bit state.
        if (state == LOW)
        {
            status = transaction.queueWrite(lowBitData, REGS_PER_LED);
        }
        else
        {
            status = transaction.queueWrite(highBitData, REGS_PER_LED);
        }
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
This expects the port bit to be configured to be constantly on or off.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to read.
\param[out] state The state of the port bit to: HIGH or LOW.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL PCA9685Device::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR bitData[REGS_PER_LED] = { 0 };        // Buffer for bit register contents


    if (portBit >= LED_COUNT)
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status && (state != HIGH) && (state != LOW))
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // If the chip is not initialized, the ports don't work.
        if (!m_chipIsInitialized)
        {
            status = FALSE;
            error = ERROR_INVALID_ENVIRONMENT;
        }
    }

    if (status)
    {
        // Set the I2C address of the PWM chip.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (portBit * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        status = transaction.queueWrite(&bitRegsAdr, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Queue reading the registers for the bit in question.
        status = transaction.queueRead(bitData, REGS_PER_LED);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Actually perform the I2C transfers specified above.
        status = transaction.execute();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // If constant OFF bit is 1, the port bit is LOW, regardless of constant ON bit.
        if ((bitData[3] & 0x10) != 0)
        {
            state = LOW;
        }
        // If constant OFF bit is 0, and constant ON bit is 1, the port bit is HIGH.
        else if ((bitData[1] & 0x10) != 0)
        {
            state = HIGH;
        }
        // If both constant state bits are zero, the port bit is not in a constant state.
        {
            status = FALSE;
            error = ERROR_INVALID_STATE;

        }
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
BOOL PCA9685Device::SetPwmDutyCycle(ULONG i2cAdr, ULONG channel, ULONG dutyCycle)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    I2cTransactionClass transaction;
    ULONGLONG tmpPulsetime = 0;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR pulseData[REGS_PER_LED] = { 0x00, 0x00, 0x00, 0x00 };    // Registers data to set pulse time


    if (channel >= LED_COUNT)
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        // Make sure the PWM chip is initialized.
        status = _InitializeChip(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the I2C address of the PWM chip.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (channel * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        status = transaction.queueWrite(&bitRegsAdr, 1);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Get the pulse high time in PWM chip terms.
        tmpPulsetime = ((((ULONGLONG)dutyCycle) * (1LL << PWM_BITS)) + 0x80000000LL) / 0x100000000LL;
        tmpPulsetime = tmpPulsetime & ((1LL << PWM_BITS) - 1LL);
        pulseData[2] = (UCHAR)(tmpPulsetime & 0xFF);
        pulseData[3] = (UCHAR)((tmpPulsetime >> 8) & 0xFF);

        // Queue sending the registers contents for the desired pulse width.
        status = transaction.queueWrite(pulseData, REGS_PER_LED);
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
This method takes any actions needed to initialize the PWM chip for use by other methods.
If the chip has already been initialized it does nothing, otherwise it sets the pulse rate
prescale value, turns on the chip and sets the mode registers for how other methods access 
the chip.
\param[in] i2cAdr The I2C address of the PWM chip.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL PCA9685Device::_InitializeChip(ULONG i2cAdr)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    // If we don't know that the chip is already initialized:
    if (!m_chipIsInitialized)
    {
        //
        // Queue a read of the chip MODE1 register to get the state of the SLEEP bit.
        //

        I2cTransactionClass transaction;
        UCHAR readBuf[1] = { 0 };                   // Buffer for reading data from chip
        UCHAR mode1RegAdr[1] = { MODE1_ADR };       // Buffer for MODE1 register address
        UCHAR mode2RegAdr[1] = { MODE2_ADR };       // Buffer for MODE2 register address
        UCHAR preScaleAdr[1] = { PRE_SCALE_ADR };   // Buffer for PRE_SCALE register address
        MODE1 mode1Reg = { 0, 0, 0, 0, 0, 1, 0, 0 }; // No sleep, auto-increment, internal clock
        MODE2 mode2Reg = { 0, 1, 1, 0, 0 };         // Drive outputs both high & low, change on ACK, non-inverted

        // Set the I2C address of the PWM chip.
        status = transaction.setAddress(i2cAdr);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Indicate this chip supports high speed I2C transfers.
            transaction.useHighSpeed();

            // Queue sending the address of the MODE1 regeister to the PWM chip.
            status = transaction.queueWrite(mode1RegAdr, sizeof(mode1Reg));
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue reading the contents of the MODE1 register.
            status = transaction.queueRead(readBuf, sizeof(readBuf));
            if (!status) { error = GetLastError(); }
        }

        //
        // If the SLEEP bit is clear, the chip has been initialize: abort the rest of the transaction.
        //

        if (status)
        {
            // Register a callback to test if the chip is initialized.
            status = transaction.queueCallback([&readBuf, &transaction]()
            {
                PMODE1 mode1Reg = (PMODE1)readBuf;
                if (mode1Reg->SLEEP == 0)   // If chip is not in sleep mode,
                {
                    transaction.abort();
                }
                return TRUE;
            }
            );
        }

        //
        // Queue writes to initialize the PWM chip.
        //

        if (status)
        {
            // Queue sending the address of the PRE_SCALE register to the PWM chip.
            status = transaction.queueWrite(preScaleAdr, sizeof(preScaleAdr), TRUE);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue sendng the prescale value.
            status = transaction.queueWrite(&m_freqPreScale, 1);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue sending the address of the MODE1 register to the PWM chip.
            status = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue sending the contents of MODE1 register.
            status = transaction.queueWrite((PUCHAR)&mode1Reg, 1);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Delay for 500 microseconds for clock to start.
            for (int i = 0; i < 5; i++)
            {
                transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
                transaction.queueRead(readBuf, sizeof(readBuf));
            }
        }

        if (status)
        {
            // Queue RESTART and sending the address of the MODE2 register to the PWM chip.
            status = transaction.queueWrite(mode2RegAdr, sizeof(mode2RegAdr), TRUE);
            if (!status) { error = GetLastError(); }
        }

        if (status)
        {
            // Queue sending the contents of MODE2 register.
            status = transaction.queueWrite((PUCHAR)&mode2Reg, 1);
            if (!status) { error = GetLastError(); }
        }

        //
        // Perform the read, decision, and writes to initialize the chip (if needed).
        //

        if (status)
        {
            // Actually perform the I2C transfers specified above.
            status = transaction.execute();
            if (!status) { error = GetLastError(); }
        }

        //
        // Whether we initialized the chip or found it initialized, we now know it is initialized.
        //

        if (status)
        {
            // Indicate chip is initialized.
            m_chipIsInitialized = TRUE;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}