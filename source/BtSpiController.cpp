// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "BtSpiController.h"
#include "BoardPins.h"

/**
\param[in] misoPin The pin number of the MISO signal.
\param[in] mosiPin The pin number of the MOSI signal.
\param[in] sckPin The pin number of the SCK signal
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::configurePins(ULONG misoPin, ULONG mosiPin, ULONG sckPin)
{
    HRESULT hr = S_OK;

    // Save the pin numbers.
    m_sckPin = sckPin;
    m_mosiPin = mosiPin;
    m_misoPin = misoPin;

    //
    // Set SCK and MOSI as outputs dedicated to SPI, and pulled LOW.
    //
    hr = g_pins.setPinMode(m_sckPin, DIRECTION_OUT, FALSE);

    if (SUCCEEDED(hr))
    {
        hr = g_pins.setPinState(m_sckPin, LOW);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pins.verifyPinFunction(m_sckPin, FUNC_SPI, BoardPinsClass::LOCK_FUNCTION);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pins.setPinMode(m_mosiPin, DIRECTION_OUT, FALSE);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pins.setPinState(m_mosiPin, LOW);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pins.verifyPinFunction(m_mosiPin, FUNC_SPI, BoardPinsClass::LOCK_FUNCTION);
    }

    //
    // Set MISO as an input dedicated to SPI.
    //
    if (SUCCEEDED(hr))
    {
        hr = g_pins.setPinMode(m_misoPin, DIRECTION_IN, FALSE);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_pins.verifyPinFunction(m_misoPin, FUNC_SPI, BoardPinsClass::LOCK_FUNCTION);
    }

    //
    // If this method failed, set all the pins back to digital I/O (ignoring any errors
    // that occur in the process.)
    //
    if (FAILED(hr))
    {
        revertPinsToGpio();
    }

    return hr;
}

/**
Initialize member variables, including the SPI Clock rate variables.
*/
BtSpiControllerClass::BtSpiControllerClass()
{
    m_hController = INVALID_HANDLE_VALUE;
    m_registers = nullptr;

    // Load values for the SPI clock generators.
    spiSpeed15mhz = { 3, 4, 4 };    // Fastest supported SPI clock on BayTrail is 15mhz
    spiSpeed12p5mhz = { 1, 1, 7 };
    spiSpeed8mhz = { 4, 5, 9 };
    spiSpeed4mhz = { 1, 1, 24 };
    spiSpeed2mhz = { 1, 1, 49 };
    spiSpeed1mhz = { 1, 1, 99 };
    spiSpeed500khz = { 1, 1, 199 };
    spiSpeed250khz = { 1, 1, 399 };
    spiSpeed125khz = { 1, 1, 799 };
    spiSpeed50khz = { 1, 1, 1999 };
    spiSpeed31k25hz = { 1, 1, 3199 };  // 31.25 khz for MIDI
    spiSpeed25khz = { 1, 1, 3999 };
    spiSpeed10khz = { 1, 4, 2499 };
    spiSpeed5khz = { 1, 8, 2499 };
    spiSpeed1khz = { 1, 40, 2499 };
}

/**
\param[in] busNumber The number of the SPI bus to open (0 or 1)
\param[in] mode The SPI mode (clock polarity and phase: 0, 1, 2 or 3)
\param[in] clockKhz The clock speed to use for the SPI bus
\param[in] dataBits The size of an SPI transfer in bits
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits)
{
    HRESULT hr = S_OK;
    
    PWCHAR deviceName = nullptr;
    PVOID baseAddress = nullptr;
    _SSCR0 sscr0;
    _SSSR sssr;


    // If this object does not yet have the SPI bus open:
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        // Get the name of the PCI device that describes the SPI controller.
        switch (busNumber)
        {
        case EXTERNAL_SPI_BUS:
            deviceName = mbmSpiDeviceName;
            break;
        default:    // Only support one SPI bus
            hr = DMAP_E_SPI_BUS_REQUESTED_DOES_NOT_EXIST;
        }

        if (SUCCEEDED(hr))
        {
            // Open the Dmap device for the SPI controller.
            hr = GetControllerBaseAddress(deviceName, m_hController, baseAddress);
            if (SUCCEEDED(hr))
            {
                m_registers = (PSPI_CONTROLLER)baseAddress;
                m_registersUpper = (PSPI_CONTROLLER_UPPER)(((PBYTE)baseAddress) + SPI_CONTROLLER_UPPER_OFFSET);
            }
        }

        if (SUCCEEDED(hr))
        {
            // We now "own" the SPI controller, intialize it.
            sscr0.ALL_BITS = 0;
            m_registers->SSCR0.ALL_BITS = sscr0.ALL_BITS;  // Disable controller

            sscr0.DSS = (dataBits - 1) & 0x0F;             // Data width ls4bits
            sscr0.EDSS = ((dataBits - 1) >> 4) & 0x01;     // Data width msbit
            sscr0.RIM = 1;                                 // Mask RX FIFO Over Run interrupts
            sscr0.TIM = 1;                                 // Mask TX FIFO Under Run interrupts
            m_registers->SSCR0.ALL_BITS = sscr0.ALL_BITS;

            m_registers->SSCR1.ALL_BITS = 0;              // Master mode, interrupts disabled

            sssr.ALL_BITS = 0;
            sssr.ROR = 1;                                  // Clear any Receive Overrun int
            sssr.PINT = 1;                                 // Clear any Peripheral Trailing Byte int
            sssr.TINT = 1;                                 // Clear any Receive Time-out int
            sssr.EOC = 1;                                  // Clear any End of Chain int
            sssr.TUR = 1;                                  // Clear any Transmit FIFO Under Run int
            sssr.BCE = 1;                                  // Clear any Bit Count Error
            m_registers->SSSR.ALL_BITS = sssr.ALL_BITS;

            hr = setMode(mode);
            
        }

        if (SUCCEEDED(hr))
        {
            hr = setClock(clockKhz);
            
        }
    }

    return hr;
}

/**
Unmap and close the SPI controller associated with this object.
*/
void BtSpiControllerClass::end()
{
    if (m_registers != nullptr)
    {
        // Disable the SPI controller.
        m_registers->SSCR0.SSE = 0;
        m_registers = nullptr;
        m_registersUpper = nullptr;
    }

    if (m_hController != INVALID_HANDLE_VALUE)
    {
        // Unmap the SPI controller.
        DmapCloseController(m_hController);
    }
}

/**
This method follows the Arduino conventions for SPI mode settings.
The SPI mode specifies the clock polarity and phase.
\param[in] mode The mode to set for the SPI bus (0, 1, 2 or 3)
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::setMode(ULONG mode)
{
    HRESULT hr = S_OK;
    
    ULONG polarity = 0;
    ULONG phase = 0;
    _SSCR1 sscr1;


    // If we don't have the controller registers mapped, fail.
    if (m_registers == nullptr)
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Determine the clock phase and polarity settings for the requested mode.
        switch (mode)
        {
        case 0:
            polarity = 0;   // Clock inactive state is low
            phase = 0;      // Sample data on active going clock edge
            break;
        case 1:
            polarity = 0;   // Clock inactive state is low
            phase = 1;      // Sample data on inactive going clock edge
            break;
        case 2:
            polarity = 1;   // Clock inactive state is high
            phase = 0;      // Sample data on active going clock edge
            break;
        case 3:
            polarity = 1;   // Click inactive state is high
            phase = 1;      // Sample data on inactive going clock edge
            break;
        default:
            hr = DMAP_E_SPI_MODE_SPECIFIED_IS_INVALID;
        }
    }

    // Set the SPI phase and polarity values in the SPI controller registers.
    if (SUCCEEDED(hr))
    {
        sscr1.ALL_BITS = m_registers->SSCR1.ALL_BITS;
        sscr1.SPO = polarity;
        sscr1.SPH = phase;
        m_registers->SSCR1.ALL_BITS = sscr1.ALL_BITS;
    }

    return hr;
}

/**
This method sets one of the SPI clock rates we support: 1 khz - 15 mhz.
\param[in] clockKhz Desired clock rate in Khz.
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::setClock(ULONG clockKhz)
{
    HRESULT hr = S_OK;
    
    PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;
    _SSCR0 sscr0;
    _PRV_CLOCK_PARAMS prvClockParams;


    // If we don't have the controller registers mapped, fail.
    if (m_registers == nullptr)
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Round down to the closest clock rate we support.
        if (clockKhz >= 15000)
        {
            pSpeed = &spiSpeed15mhz;
        }
        else if (clockKhz >= 12500)
        {
            pSpeed = &spiSpeed12p5mhz;
        }
        else if (clockKhz >= 8000)
        {
            pSpeed = &spiSpeed8mhz;
        }
        else if (clockKhz >= 4000)
        {
            pSpeed = &spiSpeed4mhz;
        }
        else if (clockKhz >= 2000)
        {
            pSpeed = &spiSpeed2mhz;
        }
        else if (clockKhz >= 1000)
        {
            pSpeed = &spiSpeed1mhz;
        }
        else if (clockKhz >= 500)
        {
            pSpeed = &spiSpeed500khz;
        }
        else if (clockKhz >= 250)
        {
            pSpeed = &spiSpeed250khz;
        }
        else if (clockKhz >= 125)
        {
            pSpeed = &spiSpeed125khz;
        }
        else if (clockKhz >= 50)
        {
            pSpeed = &spiSpeed50khz;
        }
        else if (clockKhz >= 31)
        {
            pSpeed = &spiSpeed31k25hz;
        }
        else if (clockKhz >= 25)
        {
            pSpeed = &spiSpeed25khz;
        }
        else if (clockKhz >= 10)
        {
            pSpeed = &spiSpeed10khz;
        }
        else if (clockKhz >= 5)
        {
            pSpeed = &spiSpeed5khz;
        }
        else if (clockKhz >= 1)
        {
            pSpeed = &spiSpeed1khz;
        }
        else
        {
            hr = DMAP_E_SPI_SPEED_SPECIFIED_IS_INVALID;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the clock rate.
        sscr0.ALL_BITS = m_registers->SSCR0.ALL_BITS;
        sscr0.SCR = pSpeed->SCR;
        m_registers->SSCR0.ALL_BITS = sscr0.ALL_BITS;

        prvClockParams.ALL_BITS = m_registersUpper->PRV_CLOCK_PARAMS.ALL_BITS;
        prvClockParams.M_VAL = pSpeed->M_VALUE;
        prvClockParams.N_VAL = pSpeed->N_VALUE;
        prvClockParams.CLK_UPDATE = 1;
        prvClockParams.CLK_EN = 1;
        m_registersUpper->PRV_CLOCK_PARAMS.ALL_BITS = prvClockParams.ALL_BITS;
    }
    
    return hr;
}


/**
This method transfers a buffer of data on the bus.
\param[in] dataOut The data to send on the SPI bus. If the parameter is NULL, 0's will be sent
\param[in] datIn The data received on the SPI bus. If this parameter is NULL, data in will be ignored
\param[in] bufferBytes the size of each of the buffers
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::transferBuffer(PBYTE dataOut, PBYTE dataIn, size_t bufferBytes)
{
    ULONG tempDataIn = 0;
    HRESULT hr;
    for (size_t i = 0; i < bufferBytes; i++)
    {
        hr = _transfer(dataOut ? dataOut[i] : 0, tempDataIn, 8);

        if (FAILED(hr))
        {
            break;
        }

        if (dataIn)
        {
            dataIn[i] = (BYTE)tempDataIn;
        }
    }

    return hr;
}
