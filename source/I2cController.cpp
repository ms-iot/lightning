// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "I2c.h"
#include "BoardPins.h"
#include "HiResTimer.h"

//
// I2cControllerClass methods.
//

// This method returns the esternal I2C bus pins to their default configurations.
HRESULT I2cControllerClass::revertPinsToGpio()
{
    HRESULT hr = S_OK;
    HRESULT tmpHr = S_OK;

    if (m_sclPin != INVALID_PIN_NUMBER)
    {
        hr = g_pins.verifyPinFunction(m_sclPin, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
        if (SUCCEEDED(hr))
        {
            m_sclPin = INVALID_PIN_NUMBER;
        }
    }

    if (m_sdaPin != INVALID_PIN_NUMBER)
    {
        tmpHr = g_pins.verifyPinFunction(m_sdaPin, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
        if (SUCCEEDED(hr))
        {
            hr = tmpHr;
            m_sdaPin = INVALID_PIN_NUMBER;
        }
    }

    return hr;
}


// This method maps the I2C controller if needed.
HRESULT I2cControllerClass::mapIfNeeded()
{
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        return _mapController();
    }
    else
    {
        return S_OK;
    }
}
