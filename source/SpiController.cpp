// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "SpiController.h"
#include "BoardPins.h"

/**
\param[in] busNumber The number of the SPI bus to open (0 or 1)
\param[in] mode The SPI mode to use (0-3)
\param[in] clkcKhz The SPI bit clock rate to configure in Khz (1 or greater)
\param[in] dataBits The number of data bits in each transfer (4-32)
\return HRESULT success or error code.
*/
HRESULT SpiControllerClass::begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits)
{
    HRESULT hr = S_OK;
    
    BoardPinsClass::BOARD_TYPE board;

    hr = g_pins.getBoardType(board);
    

    if (SUCCEEDED(hr) && ((dataBits < MIN_SPI_BITS) || (dataBits > MAX_SPI_BITS)))
    {
        hr = FALSE;
        SetLastError(ERROR_INVALID_DATATYPE);
    }

    if (SUCCEEDED(hr))
    {
        m_dataBits = dataBits;
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN1:
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN2:
            hr = m_quarkController._begin(busNumber, mode, clockKhz, dataBits);
            
            break;
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
            hr = m_btController._begin(busNumber, mode, clockKhz, dataBits);
            
            break;
        default:
			hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    
    return hr;
}

void SpiControllerClass::end()
{
    BoardPinsClass::BOARD_TYPE board;

    if (g_pins.getBoardType(board))
    {
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN1:
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN2:
            m_quarkController._end();
            break;
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
            m_btController._end();
            break;
        default:
            break;
        }
    }
    return;
}

/**
\param[in] clockKhz The SPI bit rate clock to set.
\return HRESULT success or error code.
*/
HRESULT SpiControllerClass::setClock(ULONG clockKhz)
{
    HRESULT hr = S_OK;
    
    BoardPinsClass::BOARD_TYPE board;

    hr = g_pins.getBoardType(board);
    

    if (SUCCEEDED(hr))
    {
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN1:
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN2:
            hr = m_quarkController._setClockRate(clockKhz);
            
            break;
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
            hr = m_btController._setClockRate(clockKhz);
            
            break;
        default:
			hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    
    return hr;
}

/**
This method follows the Arduino conventions for SPI mode settings.
The SPI mode specifies the clock polarity and phase.
\param[in] mode The mode to set for the SPI bus (0, 1, 2 or 3)
\return HRESULT success or error code.
*/
HRESULT SpiControllerClass::setMode(ULONG mode)
{
    HRESULT hr = S_OK;
    
    BoardPinsClass::BOARD_TYPE board;

    hr = g_pins.getBoardType(board);
    

    if (SUCCEEDED(hr))
    {
        switch (board)
        {
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN1:
        case BoardPinsClass::BOARD_TYPE::GALILEO_GEN2:
            hr = m_quarkController._setMode(mode);
            
            break;
        case BoardPinsClass::BOARD_TYPE::MBM_BARE:
            //hr = m_btController._setMode(mode);
            
            break;
        default:
			hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    
    return hr;
}

/// Table of bytes with bit order reversed.
const UCHAR SpiControllerClass::m_byteFlips[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

/**
Initialize member variables, including the SPI Clock rate variables.
*/
QuarkSpiControllerClass::QuarkSpiControllerClass()
{
    m_hController = INVALID_HANDLE_VALUE;
    m_controller = nullptr;

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
HRESULT QuarkSpiControllerClass::_begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits)
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
                m_controller = (PSPI_CONTROLLER)baseAddress;
            }
        }

        if (SUCCEEDED(hr))
        {
            // We now "own" the SPI controller, intialize it.
            m_controller->SSCR0.ALL_BITS = 0;              // Disable controller (and also clear other bits)
            m_controller->SSCR0.DSS = dataBits - 1;        // Use the specified data width

            m_controller->SSCR1.ALL_BITS = 0;              // Clear all register bits

            m_controller->SSSR.ROR = 1;                    // Clear any RX Overrun Status bit currently set

			hr = _setMode(mode);
        }
        if (SUCCEEDED(hr))
        {
			hr = _setClockRate(clockKhz);
        }
    }

    return hr;
}

/**
Unmap and close the SPI controller associated with this object.
*/
void QuarkSpiControllerClass::_end()
{
    if (m_controller != nullptr)
    {
        // Disable the SPI controller.
        m_controller->SSCR0.SSE = 0;
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
This method follows the Arduino conventions for SPI mode settings.
The SPI mode specifies the clock polarity and phase.
\param[in] mode The mode to set for the SPI bus (0, 1, 2 or 3)
\return HRESULT success or error code.
*/
HRESULT QuarkSpiControllerClass::_setMode(ULONG mode)
{
    HRESULT hr = S_OK;
    
    ULONG polarity = 0;
    ULONG phase = 0;


    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
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
        m_controller->SSCR1.SPO = polarity;
        m_controller->SSCR1.SPH = phase;
    }

    return hr;
}

/**
This method sets one of the SPI clock rates we support: 1 khz - 20 mhz.
\param[in] clockKhz Desired clock rate in Khz.
\return HRESULT success or error code.
*/
HRESULT QuarkSpiControllerClass::_setClockRate(ULONG clockKhz)
{
    HRESULT hr = S_OK;
    
    PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;


    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
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
        m_controller->SSCR0.SCR = pSpeed->scr;
        m_controller->DDS_RATE.DDS_CLK_RATE = pSpeed->dds_clk_rate;
    }

    
    return hr;
}

/**
Initialize member variables, including the SPI Clock rate variables.
*/
BtSpiControllerClass::BtSpiControllerClass()
{
    m_hController = INVALID_HANDLE_VALUE;
    m_controller = nullptr;

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
HRESULT BtSpiControllerClass::_begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits)
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
                m_controller = (PSPI_CONTROLLER)baseAddress;
                m_controllerUpper = (PSPI_CONTROLLER_UPPER)(((PBYTE)baseAddress) + SPI_CONTROLLER_UPPER_OFFSET);
            }
        }

        if (SUCCEEDED(hr))
        {
            // We now "own" the SPI controller, intialize it.
            sscr0.ALL_BITS = 0;
            m_controller->SSCR0.ALL_BITS = sscr0.ALL_BITS;  // Disable controller

            sscr0.DSS = (dataBits - 1) & 0x0F;             // Data width ls4bits
            sscr0.EDSS = ((dataBits - 1) >> 4) & 0x01;     // Data width msbit
            sscr0.RIM = 1;                                 // Mask RX FIFO Over Run interrupts
            sscr0.TIM = 1;                                 // Mask TX FIFO Under Run interrupts
            m_controller->SSCR0.ALL_BITS = sscr0.ALL_BITS;

            m_controller->SSCR1.ALL_BITS = 0;              // Master mode, interrupts disabled

            sssr.ALL_BITS = 0;
            sssr.ROR = 1;                                  // Clear any Receive Overrun int
            sssr.PINT = 1;                                 // Clear any Peripheral Trailing Byte int
            sssr.TINT = 1;                                 // Clear any Receive Time-out int
            sssr.EOC = 1;                                  // Clear any End of Chain int
            sssr.TUR = 1;                                  // Clear any Transmit FIFO Under Run int
            sssr.BCE = 1;                                  // Clear any Bit Count Error
            m_controller->SSSR.ALL_BITS = sssr.ALL_BITS;

            hr = _setMode(mode);
            
        }

        if (SUCCEEDED(hr))
        {
            hr = _setClockRate(clockKhz);
            
        }
    }

    return hr;
}

/**
Unmap and close the SPI controller associated with this object.
*/
void BtSpiControllerClass::_end()
{
    if (m_controller != nullptr)
    {
        // Disable the SPI controller.
        m_controller->SSCR0.SSE = 0;
        m_controller = nullptr;
        m_controllerUpper = nullptr;
    }

    if (m_hController != INVALID_HANDLE_VALUE)
    {
        // Unmap the SPI controller.
        CloseHandle(m_hController);
        m_hController = INVALID_HANDLE_VALUE;
    }
}

/**
This method follows the Arduino conventions for SPI mode settings.
The SPI mode specifies the clock polarity and phase.
\param[in] mode The mode to set for the SPI bus (0, 1, 2 or 3)
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::_setMode(ULONG mode)
{
    HRESULT hr = S_OK;
    
    ULONG polarity = 0;
    ULONG phase = 0;
    _SSCR1 sscr1;


    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
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
        sscr1.ALL_BITS = m_controller->SSCR1.ALL_BITS;
        sscr1.SPO = polarity;
        sscr1.SPH = phase;
        m_controller->SSCR1.ALL_BITS = sscr1.ALL_BITS;
    }

    return hr;
}

/**
This method sets one of the SPI clock rates we support: 1 khz - 15 mhz.
\param[in] clockKhz Desired clock rate in Khz.
\return HRESULT success or error code.
*/
HRESULT BtSpiControllerClass::_setClockRate(ULONG clockKhz)
{
    HRESULT hr = S_OK;
    
    PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;
    _SSCR0 sscr0;
    _PRV_CLOCK_PARAMS prvClockParams;


    // If we don't have the controller registers mapped, fail.
    if (m_controller == nullptr)
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
        sscr0.ALL_BITS = m_controller->SSCR0.ALL_BITS;
        sscr0.SCR = pSpeed->SCR;
        m_controller->SSCR0.ALL_BITS = sscr0.ALL_BITS;

        prvClockParams.ALL_BITS = m_controllerUpper->PRV_CLOCK_PARAMS.ALL_BITS;
        prvClockParams.M_VAL = pSpeed->M_VALUE;
        prvClockParams.N_VAL = pSpeed->N_VALUE;
        prvClockParams.CLK_UPDATE = 1;
        prvClockParams.CLK_EN = 1;
        m_controllerUpper->PRV_CLOCK_PARAMS.ALL_BITS = prvClockParams.ALL_BITS;
    }

    
    return hr;
}
