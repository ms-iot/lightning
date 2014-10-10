// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _ADC_H_
#define _ADC_H_

#include <Windows.h>
#include "ADC108S102Support.h"

class AdcClass
{
public:
    /// Constructor.
    AdcClass()
    {
        m_gen2Adc.begin();
    }

    /// Destructor.
    virtual ~AdcClass()
    {
        m_gen2Adc.end();
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
        ULONG channel;

        // Translate the pin number passed in analog channel 0-5.
        // This calculation is based on both Galileo Gen1 and Gen2 having pins
        // A0-A5 mapped to channels 0-5 on the ADC.
        channel = pin - A0;

        return m_gen2Adc.readValue(channel, value, bits);
    }

private:

    /// Gen2 ADC device.
    ADC108S102Device m_gen2Adc;

};

/// Global object used to access the A-to-D converter.
__declspec (selectany) AdcClass g_adc;

#endif  // _ADC_H_