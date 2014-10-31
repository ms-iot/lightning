// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _ADC_H_
#define _ADC_H_

#include <Windows.h>
#include "ADC108S102Support.h"
#include "AD7298Support.h"

class AdcClass
{
public:
    /// Constructor.
    AdcClass()
    {
        m_boardGeneration = 0;
    }

    /// Destructor.
    virtual ~AdcClass()
    {
        if (m_boardGeneration == 2)
        {
            m_gen2Adc.end();
        }
        else if (m_boardGeneration == 1)
        {
            m_gen1Adc.end();
        }
    }

    /// Take a reading with the ADC on the board.
    /**
    This method assumes the pin number passed in has been verified to be within 
    the range of analog inputs.
    \param[in] pin Number of Galileo GPIO pin to read with the ADC.
    \param[out] value The value read from the ADC.
    \param[out] bits The size of the reading in "value" in bits.
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL readValue(ULONG pin, ULONG & value, ULONG & bits)
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;
        ULONG channel;


        // Translate the pin number passed in analog channel 0-5.
        // This calculation is based on both Galileo Gen1 and Gen2 having pins
        // A0-A5 mapped to channels 0-5 on the ADC.
        channel = pin - A0;

        // Verify we have initialized the correct ADC.
        status = _verifyAdcInitialized();
        if (!status) { error = GetLastError(); }

        if (status)
        {
            if (m_boardGeneration == 2)
            {
                status = m_gen2Adc.readValue(channel, value, bits);
                if (!status) { error = GetLastError(); }
            }
            else
            {
                status = m_gen1Adc.readValue(channel, value, bits);
                if (!status) { error = GetLastError(); }
            }
        }

        if (!status) { SetLastError(error); }
        return status;
    }

private:

    /// The board generation for which this object has been initialized.
    ULONG m_boardGeneration;

    /// Gen2 ADC device.
    ADC108S102Device m_gen2Adc;

    /// Gen1 ADC device.
    AD7298Device m_gen1Adc;

    /// Initialize this object if it has not already been done.
    inline BOOL _verifyAdcInitialized()
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;


        // If the ADC has not yet been initialized:
        if (m_boardGeneration == 0)
        {
            // Get the board generation.
            status = g_pins.getBoardGeneration(m_boardGeneration);
            if (!status) { error = GetLastError(); }
        
            if (status)
            {
                if (m_boardGeneration == 2)
                {
                    status = m_gen2Adc.begin();
                    if (!status) { error = GetLastError(); }
                }
                else if (m_boardGeneration == 1)
                {
                    status = m_gen1Adc.begin();
                    if (!status) { error = GetLastError(); }
                }
                else
                {
                    // If we have an unrecognized board generation, indicate ADC is uninitialized.
                    m_boardGeneration = 0;
                }
            }
        }

        if (!status) { SetLastError(error); }
        return status;
    }

};

/// Global object used to access the A-to-D converter.
__declspec (selectany) AdcClass g_adc;

#endif  // _ADC_H_