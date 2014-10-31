// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _SPI_H_
#define _SPI_H_

#include <Windows.h>

#include "ArduinoCommon.h"
#include "ArduinoError.h"
#include "SpiController.h"
#include "GalileoPins.h"

// SPI clock values in KHz.
#define SPI_CLOCK_DIV2 8000
#define SPI_CLOCK_DIV4 4000
#define SPI_CLOCK_DIV8 2000
#define SPI_CLOCK_DIV16 1000
#define SPI_CLOCK_DIV32 500
#define SPI_CLOCK_DIV64 250
#define SPI_CLOCK_DIV128 125

// SPI mode values
#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

// Bit order values.
#define LSBFIRST        0x00
#define MSBFIRST        0x01

class SPIClass
{
public:
    SPIClass()
    {
        m_controller = nullptr;
        m_bitOrder = MSBFIRST;      // Default bit order is MSB First
        m_clockKHz = 4000;          // Default clock rate is 4 MHz
        m_mode = SPI_MODE0;         // Default to Mode 0
    }

    virtual ~SPIClass()
    {
        this->end();
    }

    /// Initialize the externally accessible SPI bus for use.
    void begin()
    {
        if (m_controller == nullptr)
        {
            m_controller = new SPIControllerClass;
        }

        // Set SCK and MOSI as outputs dedicated to SPI, and pulled LOW.
        if (!g_pins.verifyPinFunction(PIN_SCK, FUNC_SPI, GalileoPinsClass::LOCK_FUNCTION))
        {
            ThrowError("An error occurred configuring pinSCK for SPI use: %08x", GetLastError());
        }

        if (!g_pins.setPinState(PIN_SCK, LOW))
        {
            ThrowError("An error occurred setting pinSCK LOW: %08x", GetLastError());
        }

        if (!g_pins.setPinMode(PIN_SCK, DIRECTION_OUT, FALSE))
        {
            ThrowError("An error occurred setting pinSCK as output: %08x", GetLastError());
        }

        if (!g_pins.verifyPinFunction(PIN_MOSI, FUNC_SPI, GalileoPinsClass::LOCK_FUNCTION))
        {
            ThrowError("An error occurred configuring pinMOSI for SPI use: %08x", GetLastError());
        }

        if (!g_pins.setPinState(PIN_MOSI, LOW))
        {
            ThrowError("An error occurred setting pinMOSI LOW: %08x", GetLastError());
        }

        if (!g_pins.setPinMode(PIN_MOSI, DIRECTION_OUT, FALSE))
        {
            ThrowError("An error occurred setting pinMOSI as output: %08x", GetLastError());
        }

        // Set MISO as an input dedicated to SPI.
        if (!g_pins.verifyPinFunction(PIN_MISO, FUNC_SPI, GalileoPinsClass::LOCK_FUNCTION))
        {
            ThrowError("An error occurred configuring pinMISO for SPI use: %08x", GetLastError());
        }

        if (!g_pins.setPinMode(PIN_MISO, DIRECTION_IN, FALSE))
        {
            ThrowError("An error occurred setting pinMISO as output: %08x", GetLastError());
        }

        // Set the desired SPI bit shifting order.
        if (m_bitOrder == MSBFIRST)
        {
            m_controller->setMsbFirstBitOrder();
        }
        else
        {
            m_controller->setLsbFirstBitOrder();
        }

        // Map the SPI1 controller registers into memory.
        if (!m_controller->begin(EXTERNAL_SPI_BUS, m_mode, m_clockKHz))
        {
            ThrowError("An error occurred initializing the SPI controller: %08x", GetLastError());
        }
    }

    /// Free up the external SPI bus so its pins can be used for other functions.
    void end()
    {
        if (m_controller != nullptr)
        {
            delete m_controller;
            m_controller = nullptr;

            // Set all SPI pins as digitial I/O.
            if (!g_pins.verifyPinFunction(PIN_SCK, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION))
            {
                ThrowError("An error occurred reverting pinSCK from SPI use: %08x", GetLastError());
            }
            if (!g_pins.verifyPinFunction(PIN_MOSI, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION))
            {
                ThrowError("An error occurred reverting pinMOSI from SPI use: %08x", GetLastError());
            }
            if (!g_pins.verifyPinFunction(PIN_MISO, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION))
            {
                ThrowError("An error occurred reverting pinMISO from SPI use: %08x", GetLastError());
            }
        }
    }

    // Set the SPI bit shift order.  Expected valuse are MSBFIRST or LSBFIRST.
    void setBitOrder(int bitOrder)
    {
        if ((bitOrder != MSBFIRST) && (bitOrder != LSBFIRST))
        {
            ThrowError("SPI bit order must be MSBFIRST or LSBFIRST.");
        }
        m_bitOrder = bitOrder;

        if (m_controller != nullptr)
        {
            if (bitOrder == MSBFIRST)
            {
                m_controller->setMsbFirstBitOrder();
            }
            else
            {
                m_controller->setLsbFirstBitOrder();
            }
        }
    }

    //
    // Set the SPI clock speed.
    // For Arduino UNO compatibiltiy, this method takes one of the following defined values:
    //  SPI_CLOCK_DIV2 - Sets 8 MHz clock
    //  SPI_CLOCK_DIV4 - Sets 4 MHz clock
    //  SPI_CLOCK_DIV8 - Sets 2 MHz clock
    //  SPI_CLOCK_DIV16 - Sets 1 MHz clock
    //  SPI_CLOCK_DIV32 - Sets 500 KHz clock
    //  SPI_CLOCK_DIV64 - Sets 250 KHz clock
    //  SPI_CLOCK_DIV128 - Sets 125 KHz clock
    //
    // Primarily for testing, this method also accepts the following values:
    //  50, 25, 10, 5, 1 - Sets clock speed in KHz (50 KHz to 1 KHz).
    //
    // The actual value passed to this routine is the desired speed in KHz, with
    // an acceptable range of 125 or higher.  Underlying software layers may set a 
    // clock value that is different than the one requested to map onto supported
    // clock generator settigns, however the clock speed set will not be higher 
    // than the speed requested.
    //
    void setClockDivider(ULONG clockKHz)
    {
        m_clockKHz = clockKHz;

        if (m_controller != nullptr)
        {
            if (!m_controller->setClock(clockKHz))
            {
                ThrowError("An error occurred setting the SPI clock rate: %d", GetLastError());
            }
        }
    }

    // Set the SPI mode (clock polarity and phase).
    // Usually, the default (Mode 0) will work, so SPI mode does not need to be set.
    // The expected values are SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3.
    void setDataMode(UINT mode)
    {
        if ((mode != SPI_MODE0) && (mode != SPI_MODE1) && (mode != SPI_MODE2) && (mode != SPI_MODE3))
        {
            ThrowError("Spi Mode must be SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3.");
        }
        m_mode = mode;

        if (m_controller != nullptr)
        {
            if (!m_controller->setMode(mode))
            {
                ThrowError("An error occurred setting the SPI mode: %d", GetLastError());
            }
        }
    }

    // Transfer one byte in each direction on the SPI bus.
    inline ULONG transfer(ULONG val)
    {
        ULONG dataReturn = 0;

        if (m_controller == nullptr)
        {
            ThrowError("Can't transfer on SPI bus until an SPI.begin() has been done.");
        }

        // Transfer the data.
        if (!m_controller->transfer8(val, dataReturn))
        {
            ThrowError("An error occurred atempting to transfer SPI data: %d", GetLastError());
        }
        return dataReturn;
    }
    
private:

    // Underlying SPI Controller object that really does the work.
    SPIControllerClass *m_controller;

    // Bit order (LSBFIRST or MSBFIRST)
    ULONG m_bitOrder;

    // SPI clock rate, based on the "Clock Divider" value.
    ULONG m_clockKHz;

    // SPI mode to use.
    ULONG m_mode;
};

__declspec(selectany) SPIClass SPI;

#endif  // _SPI_H_
