// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "SpiController.h"

/**
Initialize member variables, including the SPI Clock rate variables.
*/
SPIControllerClass::SPIControllerClass()
{
    m_hController = INVALID_HANDLE_VALUE;
    m_controller = nullptr;
    m_flipBitOrder = FALSE;
    m_dataBits = 8;

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
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
BOOL SPIControllerClass::begin(ULONG busNumber, ULONG mode, ULONG clockKhz)
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;
    PWCHAR deviceName = nullptr;
    PVOID baseAddress = nullptr;


    // If this object does not yet have the SPI bus open:
    if (m_hController == INVALID_HANDLE_VALUE)
    {
        // Get the name of the PCI device that describes the SPI controller.
        switch (busNumber)
        {
        case ADC_SPI_BUS:
            deviceName = dmapSpi0DeviceName;
            break;
        case EXTERNAL_SPI_BUS:
            deviceName = dmapSpi1DeviceName;
            break;
        default:    // Only support the two SPI busses
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }

        if (status)
        {
            // Open the Dmap device for the SPI controller.
            status = GetControllerBaseAddress(deviceName, m_hController, baseAddress);
            if (!status)
            {
                error = GetLastError();
            }
            else
            {
                m_controller = (PSPI_CONTROLLER)baseAddress;
            }
        }

        if (status)
        {
            // We now "own" the SPI controller, intialize it.
            m_controller->SSCR0.ALL_BITS = 0;              // Disable controller (and also clear other bits)
            m_controller->SSCR0.DSS = m_dataBits - 1;      // Use currently set data width

            m_controller->SSCR1.ALL_BITS = 0;              // Clear all register bits

            m_controller->SSSR.ROR = 1;                    // Clear any RX Overrun Status bit currently set

            if (!setMode(mode))
            {
                status = FALSE;
                error = GetLastError();
            }
        }
        if (status)
        {
            if (!setClock(clockKhz))
            {
                status = FALSE;
                error = GetLastError();
            }
        }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
Unmap and close the SPI controller associated with this object.
*/
void SPIControllerClass::end()
{
    if (m_controller != nullptr)
    {
        // Disable the SPI controller.
        m_controller->SSCR0.SSE;
        m_controller = nullptr;
    }

    if (m_hController != INVALID_HANDLE_VALUE)
    {
        // Unmap the SPI controller.
        CloseHandle(m_hController);
        m_hController = INVALID_HANDLE_VALUE;
    }
}

/**
\param[in] clockKhz The clock speed to use for the SPI bus
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
BOOL SPIControllerClass::setClock(ULONG clockKhz)
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;

    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
    {
        status = FALSE;
        error = ERROR_NOT_READY;
    }

    if (status)
    {
        // Set the clock rate to one of the supported values.
        status = _setClockRate(clockKhz);
        if (!status)
        {
            error = GetLastError();
        }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method follows the Arduino conventions for SPI mode settings.
The SPI mode specifies the clock polarity and phase.
\param[in] mode The mode to set for the SPI bus (0, 1, 2 or 3)
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
BOOL SPIControllerClass::setMode(ULONG mode)
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;
    ULONG polarity = 0;
    ULONG phase = 0;


    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
    {
        status = FALSE;
        error = ERROR_NOT_READY;
    }

    if (status)
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
            status = FALSE;
            error = ERROR_INVALID_PARAMETER;
        }
    }

    // Set the SPI phase and polarity values in the SPI controller registers.
    if (status)
    {
        m_controller->SSCR1.SPO = polarity;
        m_controller->SSCR1.SPH = phase;
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets one of the SPI clock rates we support: 1 khz - 20 mhz.
\param[in] clockKhz Desired clock rate in Khz.
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
BOOL SPIControllerClass::_setClockRate(ULONG clockKhz)
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;
    PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;

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
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Set the clock rate.
        m_controller->SSCR0.SCR = pSpeed->scr;
        m_controller->DDS_RATE.DDS_CLK_RATE = pSpeed->dds_clk_rate;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/// Set the data width shifted on this SPI bus.
/**
Set the number of bits that are transfered in each direction during a transfer.
This method assumes the caller has verified that the SPI controller is mapped.
\param[in] bits The size of the data transfered.
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
BOOL SPIControllerClass::_setDataWidth(ULONG bits)
{
    BOOL status = TRUE;
    ULONG error = ERROR_SUCCESS;

    if ((bits < 4) || (bits > 32))
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        m_controller->SSCR0.DSS = bits - 1;
    }

    if (!status) { SetLastError(error); }
    return status;
}

