// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _SPI_CONTROLLER_H_
#define _SPI_CONTROLLER_H_

#include <Windows.h>
#include "DmapSupport.h"

#define ADC_SPI_BUS 0
#define EXTERNAL_SPI_BUS 1

#define DEFAULT_SPI_CLOCK_KHZ 4000
#define DEFAULT_SPI_MODE 0

class SPIControllerClass
{
public:
    SPIControllerClass()
    {
        m_hController = INVALID_HANDLE_VALUE;
        m_controller = nullptr;
        m_flipBitOrder = FALSE;

        // Load values for the SPI clock generators.
        spiSpeed8mhz = { 0x666666, 4 };
        spiSpeed4mhz = { 0x666666, 9};
        spiSpeed2mhz = { 0x666666, 19 };
        spiSpeed1mhz = { 0x400000, 24 };
        spiSpeed500khz = { 0x200000, 24 };
        spiSpeed250khz = { 0x200000, 49 };
        spiSpeed125khz = { 0x100000, 49 };
        spiSpeed50khz = { 0x100000, 124 };
        spiSpeed25khz = { 0x80000, 124};
        spiSpeed10khz = { 0x20000, 77 };
        spiSpeed5khz = { 0x20000, 154 };
        spiSpeed1khz = { 0x8000, 194 };
    }

    virtual ~SPIControllerClass()
    {
        this->end();
    }

    //
    // Intiialize the specified SPI but with the default mode and clock rate.
    // INPUT:
    //      busNumber - The number of the SPI bus to open (0 or 1)
    // RETURN:
    //      TRUE - Success
    //      FALSE - An error occurred.  GetLastError() returns the error code.
    //
    BOOL begin(ULONG busNumber)
    {
        return begin(busNumber, DEFAULT_SPI_MODE, DEFAULT_SPI_CLOCK_KHZ);
    }

    //
    // Initialize the specified SPI bus for use.
    // INPUT:
    //      busNumber - The number of the SPI bus to open (0 or 1)
    //      mode - Spi mode (clock polarity and phase settings): 0, 1, 2 or 3
    //      clockKhz - The clock speed to use for the SPI bus
    // RETURN:
    //      TRUE - Success
    //      FALSE - An error occurred.  GetLastError() returns the error code.
    //
    BOOL begin(ULONG busNumber, ULONG mode, ULONG clockKhz)
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
                m_controller->SSCR0.DSS = 0x07;                // Use 8-bit data

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
    
    // Unmap and close the SPI controller associated with this object.
    void end()
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

    // Method to set the default bit order: MSB First.
    void setMsbFirstBitOrder()
    {
        m_flipBitOrder = FALSE;
    }

    // Method to set the alternate bit order: LSB First.
    void setLsbFirstBitOrder()
    {
        m_flipBitOrder = TRUE;
    }

    // Method to set the SPI clock rate.
    BOOL setClock(ULONG clockKhz)
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

    // Set the SPI mode (clock polarity and phase).  This routine follows the
    // Arduino conventions for SPI mode settings.
    BOOL setMode(ULONG mode)
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

    // Method to transfer data a byte of data on the SPI bus.
    inline BOOL transfer8(ULONG dataOut, ULONG & dataIn)
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;
        ULONG txData;
        ULONG tmpData;
        ULONG rxData;
        ULONG i;

        if (m_controller == nullptr)
        {
            status = FALSE;
            error = ERROR_NOT_READY;
        }

        if (status)
        {
            // Flip the bit order if needed.
            if (m_flipBitOrder)
            {
                tmpData = dataOut;
                txData = dataOut;
                for (i = 0; i < 7; i++)
                {
                    txData = txData << 1;
                    tmpData = tmpData >> 1;
                    txData = txData | (tmpData & 0x01);
                }
                txData = txData & 0xFF;
            }
            else
            {
                txData = dataOut & 0xFF;
            }

            // Make sure the SPI bus is enabled.
            m_controller->SSCR0.SSE = 1;

            // Wait for an empty space in the FIFO.
            while (m_controller->SSSR.TNF == 0);

            // Send the data.
            m_controller->SSDR.ALL_BITS = txData;

            // Wait for data to be received.
            while (m_controller->SSSR.RNE == 0);

            // Get the received data.
            rxData = m_controller->SSDR.ALL_BITS;

            // Flip the received data bit order if needed.
            if (m_flipBitOrder)
            {
                tmpData = rxData;
                for (i = 0; i < 7; i++)
                {
                    tmpData = tmpData << 1;
                    rxData = rxData >> 1;
                    tmpData = tmpData | (rxData & 0x01);
                }
                dataIn = tmpData & 0xFF;
            }
            else
            {
                dataIn = rxData & 0xFF;
            }
        }

        if (!status)
        {
            SetLastError(error);
        }
        return status;
    }

private:

    #pragma warning(push)
    #pragma warning(disable : 4201) // Ignore nameless struct/union warnings

    // SPI Control Register 0
    typedef union {
        struct {
            ULONG DSS : 5;      // Data Size Select
            ULONG FRF : 2;      // Frame Format (Must be 0x00, other values reserved)
            ULONG SSE : 1;      // Synchronous Serial Port Enable
            ULONG SCR : 8;      // Serial Clock Rate
            ULONG _rsv : 16;    // Reserved
        };
        ULONG ALL_BITS;
    } _SSCR0;

    // SPI Control Register 1
    typedef union {
        struct {
            ULONG RIE : 1;      // Receive FIFO Interrupt Enable
            ULONG TIE : 1;      // Transmit FIFO Interrupt Enable
            ULONG LBM : 1;      // Loop Back Mode
            ULONG SPO : 1;      // Serial Clock Polarity
            ULONG SPH : 1;      // Serial Clock Phase
            ULONG _rsv1 : 1;    // Reserved
            ULONG TFT : 5;      // Transmit FIFO Interrupt Threshold
            ULONG RFT : 5;      // Receive FIFO Interrupt Threshold
            ULONG EFWR : 1;     // Enable FIFO Write/Read Function
            ULONG STRF : 1;     // Select FIFO for Enable FIFO Write/Read
            ULONG _rsv2 : 14;   // Reserved
        };
        ULONG ALL_BITS;
    } _SSCR1;

    // SPI Status Register
    typedef union {
        struct {
            ULONG ALT_FRM : 2;  // Alternative Frame (not supported)
            ULONG TNF : 1;      // Transmit FIFO Not Full Flag
            ULONG RNE : 1;      // Receive FIFO Not Empty Flag
            ULONG BSY : 1;      // SPI Busy Flag
            ULONG TFS : 1;      // Transmit FIFO Service Request Flag
            ULONG RFS : 1;      // Receive FIFO Service Request Flag
            ULONG ROR : 1;      // Receiver Overrun Status
            ULONG TFL : 5;      // Transmit FIFO Level
            ULONG RFL : 5;      // Receive FIFO Level
            ULONG _rsv : 14;    // Reserved
        };
        ULONG ALL_BITS;
    } _SSSR;

    // SPI Data Register
    typedef union {
        struct {
            ULONG DATA8 : 8;    // We currently only support 8-bit data
            ULONG _unused : 24; // Unused bits with 8-bit data
        };
        ULONG ALL_BITS;
    } _SSDR;

    // DDS Clock Rate Register
    typedef union {
        struct {
            ULONG DDS_CLK_RATE : 24;
            ULONG _rsv : 8;
        };
        ULONG ALL_BITS;
    } _DDS_RATE;

    #pragma warning( pop )

    // Layout of the Quark SPI Controller registers in memory.
    typedef struct _SPI_CONTROLLER {
        volatile _SSCR0     SSCR0;          // 0x00 - SPI Control Register 0
        volatile _SSCR1     SSCR1;          // 0x04 - SPI Control Register 1
        volatile _SSSR      SSSR;           // 0x08 - SPI Status Register
        ULONG               _reserved1;     // 0x0C - 0x0F
        volatile _SSDR      SSDR;           // 0x10 - SPI Data Register
        ULONG               _reserved2[5];  // 0x14 - 0x27
        volatile _DDS_RATE  DDS_RATE;       // 0x28 - DDS Clock Rate Register
    } SPI_CONTROLLER, *PSPI_CONTROLLER;

    // Struct used to specify an SPI bus speed.
    typedef struct _SPI_BUS_SPEED {
        ULONG dds_clk_rate;                 // Value for DDS_CLK_RATE register
        ULONG scr;                          // Value for SSCR0.SCR bit field
    } SPI_BUS_SPEED, *PSPI_BUS_SPEED;

    // Spi bus speed values.
    SPI_BUS_SPEED spiSpeed8mhz;
    SPI_BUS_SPEED spiSpeed4mhz;
    SPI_BUS_SPEED spiSpeed2mhz;
    SPI_BUS_SPEED spiSpeed1mhz;
    SPI_BUS_SPEED spiSpeed500khz;
    SPI_BUS_SPEED spiSpeed250khz;
    SPI_BUS_SPEED spiSpeed125khz;
    SPI_BUS_SPEED spiSpeed50khz;
    SPI_BUS_SPEED spiSpeed25khz;
    SPI_BUS_SPEED spiSpeed10khz;
    SPI_BUS_SPEED spiSpeed5khz;
    SPI_BUS_SPEED spiSpeed1khz;

    // Method to set one of the clock rates we support.
    BOOL _setClockRate(ULONG clockKhz)
    {
        BOOL status = TRUE;
        ULONG error = ERROR_SUCCESS;
        PSPI_BUS_SPEED pSpeed = &spiSpeed4mhz;

        // Round down to the closest clock rate we support.
        if (clockKhz >= 8000)
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

        if (!status)
        {
            SetLastError(error);
        }
        return status;

    }

    // Handle to the device that can be used to map SPI controller registers 
    // into user-mode address space.
    HANDLE m_hController;

    // Pointer to the object used to address the SPI Controller registers after
    // they are mapped into this process' address space.
    PSPI_CONTROLLER m_controller;

    // If TRUE invert the data before/after transfer (Controller only supports MSB first)
    BOOL m_flipBitOrder;
};

#endif  // _SPI_CONTROLLER_H_