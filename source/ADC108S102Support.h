// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _ADC108S102_SUPPORT_H_
#define _ADC108S102_SUPPORT_H_

#include <Windows.h>
#include "SpiController.h"
#include "GpioController.h"

class ADC108S102Device
{
public:
    /// Constructor.
    ADC108S102Device()
    {
        m_csFabricBit = 0;
    }

    /// Destructor.
    virtual ~ADC108S102Device()
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
        status = m_spi.begin(ADC_SPI_BUS, 3, 12500);
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

    /// Take a reading with the ADC used on the Gen2 board.
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
        ULONG dataOut = 0;
        ULONG dataIn = 0;


        // Make sure the channel number is in range.
        if (channel >= ADC_CHANNELS)
        {
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        if (status)
        {
            // Prepare to send the channel number to the SPI controller.
            dataOut = channel << CHAN_SHIFT;

            // Perform a conversion and get the result.
            g_fabricGpio.setPinState(m_csFabricBit, LOW);       // Make ADC chip select active
            if (!status) { error = GetLastError(); }
            else
            {
                status = m_spi.transfer32(dataOut, dataIn);
                if (!status) { error = GetLastError(); }
                g_fabricGpio.setPinState(m_csFabricBit, HIGH);  // Make ADC chip select inactive
            }
        }

        if (status)
        {
            // Extract the reading from the data sent back from the ADC.
            value = (dataIn >> DATA_SHIFT) & ((1 << ADC_BITS) - 1);
            bits = ADC_BITS;
        }

        if (!status) { SetLastError(error); }
        return status;
    }

private:
    /// The number of channels on the ADC.
    const ULONG ADC_CHANNELS = 8;

    /// The number of bits in an ADC conversion.
    const ULONG ADC_BITS = 10;

    /// The shift amount to put the channel number into a 32-bit longword.
    const ULONG CHAN_SHIFT = 27;

    /// The shift amount to right justify the data read from the ADC.
    const ULONG DATA_SHIFT = 2;

    /// The SPI Controller object used to talk to the ADC.
    SPIControllerClass m_spi;

    /// The Fabric GPIO bit that controls the chip select signal.
    ULONG m_csFabricBit;
};

#endif  // _ADC108S102_SUPPORT_H_