// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "QuarkSpiController.h"
#include "BoardPins.h"

/**
\param[in] misoPin The pin number of the MISO signal.
\param[in] mosiPin The pin number of the MOSI signal.
\param[in] sckPin The pin number of the SCK signal
\return HRESULT success or error code.
*/
HRESULT QuarkSpiControllerClass::configurePins(ULONG misoPin, ULONG mosiPin, ULONG sckPin)
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
QuarkSpiControllerClass::QuarkSpiControllerClass()
{
    m_hController = INVALID_HANDLE_VALUE;
    m_registers = nullptr;

    // Load values for the SPI clock generators.
    spiSpeed20mhz = { 0x666666, 1 };    // 20 mhz for Gen1 ADC
    spiSpeed12p5mhz = { 0x200000, 0 };  // 12.5 mhz for Gen2 ADC
    spiSpeed8mhz = { 0x666666, 4 };
    spiSpeed4mhz = { 0x666666, 9 };
    spiSpeed2mhz = { 0x666666, 19 };
    spiSpeed1mhz = { 0x400000, 24 };
    spiSpeed500khz = { 0x200000, 24 };
    spiSpeed250khz = { 0x200000, 49 };
    spiSpeed125khz = { 0x100000, 49 };
    spiSpeed50khz = { 0x100000, 124 };
    spiSpeed31k25hz = { 0x28f5C, 31 };  // 31.25 khz for MIDI
    spiSpeed25khz = { 0x80000, 124 };
    spiSpeed10khz = { 0x20000, 77 };
    spiSpeed5khz = { 0x20000, 154 };
    spiSpeed1khz = { 0x8000, 194 };
}

/**
\param[in] busNumber The number of the SPI bus to open (0 or 1)
\param[in] mode The SPI mode (clock polarity and phase: 0, 1, 2 or 3)
\param[in] clockKhz The clock speed to use for the SPI bus
\param[in] dataBits The size of an SPI transfer in bits
\return HRESULT success or error code.
*/
HRESULT QuarkSpiControllerClass::begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits)
{
    HRESULT hr = S_OK;
    
    PWCHAR deviceName = nullptr;
    PVOID baseAddress = nullptr;


    // If this object does not yet have the SPI bus open:
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        // Get the name of the PCI device that describes the SPI controller.
        switch (busNumber)
        {
        case ADC_SPI_BUS:
            deviceName = galileoSpi0DeviceName;
            break;
        case EXTERNAL_SPI_BUS:
            deviceName = galileoSpi1DeviceName;
            break;
        default:    // Only support the two SPI busses
            hr = DMAP_E_SPI_BUS_REQUESTED_DOES_NOT_EXIST;
        }

        if (SUCCEEDED(hr))
        {
            // Open the Dmap device for the SPI controller.
            hr = GetControllerBaseAddress(deviceName, m_hController, baseAddress);

            if (SUCCEEDED(hr))
            {
                m_registers = (PSPI_CONTROLLER)baseAddress;
            }
        }

        if (SUCCEEDED(hr))
        {
            // We now "own" the SPI controller, intialize it.
            m_registers->SSCR0.ALL_BITS = 0;              // Disable controller (and also clear other bits)
            m_registers->SSCR0.DSS = dataBits - 1;        // Use the specified data width

            m_registers->SSCR1.ALL_BITS = 0;              // Clear all register bits

            m_registers->SSSR.ROR = 1;                    // Clear any RX Overrun Status bit currently set

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
void QuarkSpiControllerClass::end()
{
    if (m_registers != nullptr)
    {
        // Disable the SPI controller.
        m_registers->SSCR0.SSE = 0;
        m_registers = nullptr;
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
HRESULT QuarkSpiControllerClass::setMode(ULONG mode)
{
    HRESULT hr = S_OK;
    
    ULONG polarity = 0;
    ULONG phase = 0;


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
        m_registers->SSCR1.SPO = polarity;
        m_registers->SSCR1.SPH = phase;
    }

    return hr;
}

/**
This method sets one of the SPI clock rates we support: 1 khz - 20 mhz.
\param[in] clockKhz Desired clock rate in Khz.
\return HRESULT success or error code.
*/
HRESULT QuarkSpiControllerClass::setClock(ULONG clockKhz)
{
    HRESULT hr = S_OK;
    
    PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;


    // If we don't have the controller registers mapped, fail.
    if (m_registers == nullptr)
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Round down to the closest clock rate we support.
        if (clockKhz >= 20000)
        {
            pSpeed = &spiSpeed20mhz;
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
        m_registers->SSCR0.SCR = pSpeed->scr;
        m_registers->DDS_RATE.DDS_CLK_RATE = pSpeed->dds_clk_rate;
    }
 
    return hr;
}
