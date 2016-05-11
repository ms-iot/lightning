// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "PCA9685Support.h"
#include "ExpanderDefs.h"
#include "I2c.h"
#include "ErrorCodes.h"
#include "ArduinoCommon.h"

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
\return HRESULT success or error code.
*/
HRESULT PCA9685Device::SetBitState(ULONG i2cAdr, ULONG portBit, ULONG state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR lowBitData[REGS_PER_LED] = { 0x00, 0x00, 0x00, 0x10 };    // Registers data to set bit low
    UCHAR highBitData[REGS_PER_LED] = { 0x00, 0x10, 0x00, 0x00 };   // Registers data to set bit high

    if (portBit >= LED_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr) && (state != HIGH) && (state != LOW))
    {
        hr = DMAP_E_INVALID_PIN_STATE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        // Make sure the PWM chip is initialized.
        hr = _InitializeChip(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the PWM chip.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (portBit * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        hr = transaction.queueWrite(&bitRegsAdr, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Queue sending the registers contents to set the specified bit state.
        if (state == LOW)
        {
            hr = transaction.queueWrite(lowBitData, REGS_PER_LED);
        }
        else
        {
            hr = transaction.queueWrite(highBitData, REGS_PER_LED);
        }
        
    }

    if (SUCCEEDED(hr))
    {
        // Actually perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

/**
This expects the port bit to be configured to be constantly on or off.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] portBit The number of the port bit to read.
\param[out] state The state of the port bit: HIGH or LOW.
\return HRESULT success or error code.
*/
HRESULT PCA9685Device::GetBitState(ULONG i2cAdr, ULONG portBit, ULONG & state)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR bitData[REGS_PER_LED] = { 0 };        // Buffer for bit register contents


    if (portBit >= LED_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Make sure the PWM chip is initialized.
        hr = _InitializeChip(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the PWM chip.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (portBit * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        hr = transaction.queueWrite(&bitRegsAdr, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Queue reading the registers for the bit in question.
        hr = transaction.queueRead(bitData, REGS_PER_LED);
    }

    if (SUCCEEDED(hr))
    {
        // Actually perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }

    if (SUCCEEDED(hr))
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
            hr = DMAP_E_GPIO_PIN_IS_SET_TO_PWM;
        }
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
HRESULT PCA9685Device::SetPwmDutyCycle(ULONG i2cAdr, ULONG channel, ULONG dutyCycle)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONGLONG tmpPulsetime = 0;
    UCHAR bitRegsAdr = 0;                       // Address of start of registers for bit in question
    UCHAR pulseData[REGS_PER_LED] = { 0x00, 0x00, 0x00, 0x00 };    // Registers data to set pulse time


    if (channel >= LED_COUNT)
    {
        hr = DMAP_E_INVALID_PORT_BIT_FOR_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        // Make sure the PWM chip is initialized.
        hr = _InitializeChip(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the PWM chip.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the address of the first register for the port in question.
        bitRegsAdr = (UCHAR)(LEDS_BASE_ADR + (channel * REGS_PER_LED));

        // Queue sending the base address of the port registers to the chip.
        hr = transaction.queueWrite(&bitRegsAdr, 1);
    }

    if (SUCCEEDED(hr))
    {
        // Get the pulse high time in PWM chip terms.
        tmpPulsetime = ((((ULONGLONG)dutyCycle) * (1LL << PWM_BITS)) + 0x80000000LL) / 0x100000000LL;
        tmpPulsetime = tmpPulsetime & ((1LL << PWM_BITS) - 1LL);
        pulseData[2] = (UCHAR)(tmpPulsetime & 0xFF);
        pulseData[3] = (UCHAR)((tmpPulsetime >> 8) & 0xFF);

        // Queue sending the registers contents for the desired pulse width.
        hr = transaction.queueWrite(pulseData, REGS_PER_LED);
    }

    if (SUCCEEDED(hr))
    {
        // Actually perform the I2C transfers specified above.
        hr = transaction.execute(g_i2c.getController());
    }
    
    return hr;
}

/**
Set the pulse repetition rate for the PWM channels on the specified chip.
\param[in] i2cAdr The I2C address of the PWM chip.
\param[in] frequency The desired PWM pulse repetition rate in pulses per second.
\return HRESULT success or error code.
*/
HRESULT PCA9685Device::SetPwmFrequency(ULONG i2cAdr, ULONG frequency)
{
    HRESULT hr = S_OK;
    I2cTransactionClass transaction;
    ULONG preScale;
    UCHAR readBuf[1] = { 0 };                       // Buffer for reading data from chip
    UCHAR mode1RegAdr[1] = { MODE1_ADR };           // Buffer for MODE1 register address
    UCHAR preScaleAdr[1] = { PRE_SCALE_ADR };       // Buffer for PRE_SCALE register address
    MODE1 mode1Sleep = { 0, 0, 0, 0, 1, 1, 0, 0 };  // Sleep, auto-increment, internal clock
    MODE1 mode1Run = { 0, 0, 0, 0, 0, 1, 0, 0 };    // No sleep, auto-increment, internal clock


                                                    // Make sure the PWM chip is initialized.
    hr = _InitializeChip(i2cAdr);

    if (SUCCEEDED(hr))
    {
        // Set the I2C address of the PWM chip.
        hr = transaction.setAddress(i2cAdr);
    }

    if (SUCCEEDED(hr))
    {
        // Indicate this chip supports high speed I2C transfers.
        transaction.useHighSpeed();

        // Calculate the nearest prescale value for the requested pulse rate.
        if (frequency < 24)
        {
            preScale = 0xFF;
        }
        else
        {
            // From PCA9685 datasheet: prescale = round(25,000,000 / (4096 * pulse_rate)) - 1
            preScale = (((25000000 + ((4096 * frequency) / 2))) / (4096 * frequency)) - 1;
        }
        preScale = preScale & 0xFF;
    }

    // If we need to set a new prescale value.
    if (SUCCEEDED(hr) && (m_freqPreScale != preScale))
    {
        // Queue a write to set the Sleep bit (so we can change the PWM frequency).
        hr = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr));
        if (SUCCEEDED(hr))
        {
            hr = transaction.queueWrite((PUCHAR)&mode1Sleep, 1);
        }

        // Queue a write to set the frequency prescale value.
        if (SUCCEEDED(hr))
        {
            hr = transaction.queueWrite(preScaleAdr, sizeof(preScaleAdr), TRUE);
        }
        if (SUCCEEDED(hr))
        {
            hr = transaction.queueWrite((PUCHAR)&preScale, 1);
        }

        // Queue a write to clear the Sleep bit.
        if (SUCCEEDED(hr))
        {
            hr = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
        }
        if (SUCCEEDED(hr))
        {
            hr = transaction.queueWrite((PUCHAR)&mode1Run, 1);
        }

        // Delay for 500 microseconds for clock to start.
        for (int i = 0; SUCCEEDED(hr) && (i < 5); i++)
        {
            hr = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
            if (SUCCEEDED(hr))
            {
                hr = transaction.queueRead(readBuf, sizeof(readBuf));
            }
        }

        if (SUCCEEDED(hr))
        {
            // Actually perform the I2C transfers specified above.
            hr = transaction.execute(g_i2c.getController());
        }

        // Record the prescale value just set.
        if (SUCCEEDED(hr))
        {
            m_freqPreScale = (UCHAR)preScale;
        }
    }

    return hr;
}

/**
Get the actual pulse repetition rate for the PWM channels on the specified chip.
\param[in] i2cAdr The I2C address of the PWM chip.
\return The approximate actual pulse repetion rate of the PWM channels.
*/
ULONG PCA9685Device::GetActualPwmFrequency(ULONG i2cAdr)
{
    HRESULT hr = S_OK;
    ULONG frequency;
    ULONG divisor;

    // From PCA9685 datasheet: prescale = round(25,000,000 / (4096 * pulse_rate)) - 1
    // so pulse_rate = round( 25,000,000 / ((prescale + 1) * 4096) )

    divisor = (m_freqPreScale + 1) * 4096;
    frequency = (25000000 + (divisor / 2)) / divisor;

    return frequency;
}

/**
This method takes any actions needed to initialize the PWM chip for use by other methods.
If the chip has already been initialized it does nothing, otherwise it sets the pulse rate
prescale value, turns on the chip and sets the mode registers for how other methods access 
the chip.
\param[in] i2cAdr The I2C address of the PWM chip.
\return HRESULT success or error code.
*/
HRESULT PCA9685Device::_InitializeChip(ULONG i2cAdr)
{
    HRESULT hr = S_OK;

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
        hr = transaction.setAddress(i2cAdr);

        if (SUCCEEDED(hr))
        {
            // Indicate this chip supports high speed I2C transfers.
            transaction.useHighSpeed();

            // Queue sending the address of the MODE1 regeister to the PWM chip.
            hr = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr));
        }

        if (SUCCEEDED(hr))
        {
            // Queue reading the contents of the MODE1 register.
            hr = transaction.queueRead(readBuf, sizeof(readBuf));
        }

        //
        // If the SLEEP bit is clear, the chip has been initialize: abort the rest of the transaction.
        //

        if (SUCCEEDED(hr))
        {
            // Register a callback to test if the chip is initialized.
            hr = transaction.queueCallback([&readBuf, &transaction]()
            {
                PMODE1 mode1RegPtr = (PMODE1)readBuf;
                if (mode1RegPtr->SLEEP == 0)   // If chip is not in sleep mode,
                {
                    transaction.abort();
                }
                return S_OK;
            }
            );
        }

        //
        // Queue writes to initialize the PWM chip.
        //

        if (SUCCEEDED(hr))
        {
            // Queue sending the address of the PRE_SCALE register to the PWM chip.
            hr = transaction.queueWrite(preScaleAdr, sizeof(preScaleAdr), TRUE);
        }

        if (SUCCEEDED(hr))
        {
            // Queue sendng the prescale value.
            hr = transaction.queueWrite(&m_freqPreScale, 1);
        }

        if (SUCCEEDED(hr))
        {
            // Queue sending the address of the MODE1 register to the PWM chip.
            hr = transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
        }

        if (SUCCEEDED(hr))
        {
            // Queue sending the contents of MODE1 register.
            hr = transaction.queueWrite((PUCHAR)&mode1Reg, 1);
        }

        if (SUCCEEDED(hr))
        {
            // Delay for 500 microseconds for clock to start.
            for (int i = 0; i < 5; i++)
            {
                transaction.queueWrite(mode1RegAdr, sizeof(mode1RegAdr), TRUE);
                transaction.queueRead(readBuf, sizeof(readBuf));
            }
        }

        if (SUCCEEDED(hr))
        {
            // Queue RESTART and sending the address of the MODE2 register to the PWM chip.
            hr = transaction.queueWrite(mode2RegAdr, sizeof(mode2RegAdr), TRUE);
        }

        if (SUCCEEDED(hr))
        {
            // Queue sending the contents of MODE2 register.
            hr = transaction.queueWrite((PUCHAR)&mode2Reg, 1);
        }

        //
        // Perform the read, decision, and writes to initialize the chip (if needed).
        //

        if (SUCCEEDED(hr))
        {
            // Actually perform the I2C transfers specified above.
            hr = transaction.execute(g_i2c.getController());
        }

        //
        // Whether we initialized the chip or found it initialized, we now know it is initialized.
        //

        if (SUCCEEDED(hr))
        {
            // Indicate chip is initialized.
            m_chipIsInitialized = TRUE;
        }
    }
    
    return hr;
}