// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _AD7298_SUPPORT_H_
#define _AD7298_SUPPORT_H_

#include <Windows.h>
#include "SpiController.h"
#include "GpioController.h"

class AD7298Device
{
public:
    /// Constructor.
    AD7298Device()
    {
    }

    /// Destructor.
    virtual ~AD7298Device()
    {
    }

    /// Prepare to use this ADC.
    /**
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL begin()
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;

        // Prepare to use the controller for the ADC's SPI controller.
        status = m_spi.begin(ADC_SPI_BUS, 2, 20000);
        if (!status)  { error = GetLastError(); }

        if (status)
        {
            // Make the fabric GPIO bit that drives the ADC chip select signal an output.
            status = g_fabricGpio.setPinDirection(m_csFabricBit, DIRECTION_OUT);
            if (!status)  { error = GetLastError(); }
        }

        if (!status) { SetLastError(error); }
        return status;
    }

    /// Release the ADC.
    inline void end()
    {
        // Release the ADC SPI bus.
        m_spi.end();
    }

    /// Take a reading with the ADC used on the Gen1 board.
    /**
    \param[in] channel Number of channel on ADC to read.
    \param[out] value The value read from the ADC.
    \param[out] bits The size of the reading in "value" in bits.
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL readValue(ULONG channel, ULONG & value, ULONG & bits)
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;
        ULONG dataOut;
        ULONG dataIn;
        ULONG chanIn;
        USHORT chanMask;
        CMD_REG cmdReg;


        // Make sure the channel number is in range.
        if (channel >= ADC_CHANNELS)
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        //
        // Jog the ADC twice to bring it out of any unresponsive state (such as Partial
        // Power-Down) that the shutdown of a previous program may have left it in.
        //
        if (status)
        {
            // Build ADC command register contents with Partial Power-Down bit clear.
            cmdReg.ALL_BITS = 0;
            cmdReg.WRITE = 1;
            dataOut = (ULONG)cmdReg.ALL_BITS;

            status = g_fabricGpio.setPinState(m_csFabricBit, LOW);  // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                // Write the ADC command register.
                status = m_spi.transfer16(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);      // Make ADC chip select inactive
            }
        }

        if (status)
        {
            // Build ADC command register contents with Partial Power-Down bit clear.
            cmdReg.ALL_BITS = 0;
            cmdReg.WRITE = 1;
            dataOut = (ULONG)cmdReg.ALL_BITS;

            status = g_fabricGpio.setPinState(m_csFabricBit, LOW);  // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                // Write the ADC command register.
                status = m_spi.transfer16(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);      // Make ADC chip select inactive
            }
        }

        //
        // Tell the ADC which channel we want to read.
        //

        if (status)
        {
            // Build ADC command register contents with bit set for the channel we want to read.
            chanMask = 0x0080 >> channel;
            cmdReg.ALL_BITS = 0;
            cmdReg.CHAN = chanMask;
            cmdReg.WRITE = 1;
            dataOut = (ULONG)cmdReg.ALL_BITS;

            status = g_fabricGpio.setPinState(m_csFabricBit, LOW);  // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                // Send the channel information to the ADC.
                status = m_spi.transfer16(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);      // Make ADC chip select inactive
            }
        }

        //
        // Perform the ADC conversion for the specified channel.
        //

        if (status)
        {
            // Shift out 16 bits to perform the conversion.
            dataOut = 0;
            status = g_fabricGpio.setPinState(m_csFabricBit, LOW);  // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                status = m_spi.transfer16(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);      // Make ADC chip select inactive
            }
        }

        //
        // Get the conversion result from the ADC.
        //

        if (status)
        {
            // Build ADC command register contents with Partial Power-Down bit set.
            cmdReg.ALL_BITS = 0;
            cmdReg.WRITE = 1;
            cmdReg.PPD = 1;
            dataOut = (ULONG)cmdReg.ALL_BITS;

            status = g_fabricGpio.setPinState(m_csFabricBit, LOW);  // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                // Write the ADC command register and shift out the conversion result.
                status = m_spi.transfer16(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);      // Make ADC chip select inactive
            }
        }

        //
        // Verify we got data for the right channel and pass it back to the caller.
        //

        if (status)
        {
            chanIn = (dataIn >> ADC_BITS) & ((1 << ADC_CHAN_BITS) - 1);

            if (chanIn != channel)
            {
                status = FALSE;
                error = ERROR_NO_DATA_DETECTED;
            }
        }

        if (status)
        {
            value = dataIn & ((1 << ADC_BITS) - 1);
            bits = ADC_BITS;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

private:

    /// Struct for ADC Control Register contents.
    typedef union {
        struct {
            USHORT PPD : 1;
            USHORT TsenceAvg : 1;
            USHORT EXT_REF : 1;
            USHORT dontcare : 2;
            USHORT Tsense : 1;
            USHORT CH7 : 1;
            USHORT CH6 : 1;
            USHORT CH5 : 1;
            USHORT CH4 : 1;
            USHORT CH3 : 1;
            USHORT CH2 : 1;
            USHORT CH1 : 1;
            USHORT CH0 : 1;
            USHORT REPEAT : 1;
            USHORT WRITE : 1;
        };
        struct {
            USHORT pad1 : 6;
            USHORT CHAN : 8;
            USHORT pad2 : 2;
        };
        USHORT ALL_BITS;
    } CMD_REG, *PCMD_REG;

    /// The number of channels on the ADC.
    const ULONG ADC_CHANNELS = 8;

    /// The number of bits in an ADC conversion.
    const ULONG ADC_BITS = 12;

    /// The number of channel number bits returned with an ADC conversion.
    const ULONG ADC_CHAN_BITS = 4;

    /// The SPI Controller object used to talk to the ADC.
    SPIControllerClass m_spi;

    /// The Fabric GPIO bit that controls the chip select signal.
    const ULONG m_csFabricBit = 0;
};

#endif  // _AD7298_SUPPORT_H_