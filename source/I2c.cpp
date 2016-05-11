// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "I2c.h"
#include "BoardPins.h"
#include "BcmI2cController.h"
#include "BtI2cController.h"
#include "HiResTimer.h"


// 
// Global extern exports
//
I2cClass g_i2c(EXTERNAL_I2C_BUS);
I2cClass g_i2c2nd(SECOND_EXTERNAL_I2C_BUS);

//
// I2cClass methods.
//

/// Prepare to use the I2C controller associated with this object.
HRESULT I2cClass::begin()
{
    HRESULT hr;
    BoardPinsClass::BOARD_TYPE board;

    EnterCriticalSection(&m_lock);

    hr = g_pins.getBoardType(board);

    // Create and initialize the I2C Controller object if we don't already have one.
    if (m_controller == nullptr)
    {
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::PI2_BARE:
            m_controller = new BcmI2cControllerClass;
            if (m_busNumber == SECOND_EXTERNAL_I2C_BUS)
            {
                hr = m_controller->configurePins(BARE_PI2_PIN_I2C0_DAT, BARE_PI2_PIN_I2C0_CLK);
            }
            else
            {
                hr = m_controller->configurePins(BARE_PI2_PIN_I2C1_DAT, BARE_PI2_PIN_I2C1_CLK);
            }
            break;
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
        case BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE:
            m_controller = new BtI2cControllerClass;
            hr = m_controller->configurePins(BARE_MBM_PIN_I2C_DAT, BARE_MBM_PIN_I2C_CLK);
            break;
        default:
            hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    hr = m_controller->begin(m_busNumber);

    if (SUCCEEDED(hr))
    {
        m_refCount++;
    }

    LeaveCriticalSection(&m_lock);

    return hr;
}

/// Finish using the I2C controller associated with this object.
void I2cClass::end()
{
    EnterCriticalSection(&m_lock);

    if (m_refCount > 0)
    {
        m_refCount--;
        if (m_refCount == 0)
        {
            delete m_controller;
            m_controller = nullptr;
        }
    }

    LeaveCriticalSection(&m_lock);
}
