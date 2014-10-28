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
    /// Constructor.
    SPIControllerClass();

    virtual ~SPIControllerClass()
    {
        this->end();
    }

    /// Initialize the specified SPI bus, using the default mode and clock rate.
    /**
    \param[in] busNumber The number of the SPI bus to open (0 or 1)
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    BOOL begin(ULONG busNumber)
    {
        return begin(busNumber, DEFAULT_SPI_MODE, DEFAULT_SPI_CLOCK_KHZ);
    }

    /// Initialize the specified SPI bus for use.
    BOOL begin(ULONG busNumber, ULONG mode, ULONG clockKhz);

    /// Finish using an SPI controller.
    void end();

    /// Method to set the default bit order: MSB First.
    void setMsbFirstBitOrder()
    {
        m_flipBitOrder = FALSE;
    }

    /// Method to set the alternate bit order: LSB First.
    void setLsbFirstBitOrder()
    {
        m_flipBitOrder = TRUE;
    }

    /// Set the SPI clock rate.
    BOOL setClock(ULONG clockKhz);

    /// Set the SPI mode (clock polarity and phase).
    BOOL setMode(ULONG mode);

    /// Transfer a byte of data on the SPI bus.
    /**
    \param[in] dataOut A byte of data to send on the SPI bus
    \param[out] datIn The byte of data received on the SPI bus
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL transfer8(ULONG dataOut, ULONG & dataIn)
    {
        return transfer(dataOut, dataIn, 8);
    }

    /// Transfer a word of data on the SPI bus.
    /**
    \param[in] dataOut A word of data to send on the SPI bus
    \param[out] datIn The word of data received on the SPI bus
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL transfer16(ULONG dataOut, ULONG & dataIn)
    {
        return transfer(dataOut, dataIn, 16);
    }

    /// Transfer a longword of data on the SPI bus.
    /**
    \param[in] dataOut A longword of data to send on the SPI bus
    \param[out] datIn The longword of data received on the SPI bus
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL transfer32(ULONG dataOut, ULONG & dataIn)
    {
        return transfer(dataOut, dataIn, 32);
    }

    /// Perform a transfer on the SPI bus.
    /**
    \param[in] dataOut The data to send on the SPI bus
    \param[out] datIn The data received on the SPI bus
    \return TRUE, success. FALSE, failure, GetLastError() returns the error code.
    */
    inline BOOL transfer(ULONG dataOut, ULONG & dataIn, ULONG bits);

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
    SPI_BUS_SPEED spiSpeed20mhz;
    SPI_BUS_SPEED spiSpeed12p5mhz;
    SPI_BUS_SPEED spiSpeed8mhz;
    SPI_BUS_SPEED spiSpeed4mhz;
    SPI_BUS_SPEED spiSpeed2mhz;
    SPI_BUS_SPEED spiSpeed1mhz;
    SPI_BUS_SPEED spiSpeed500khz;
    SPI_BUS_SPEED spiSpeed250khz;
    SPI_BUS_SPEED spiSpeed125khz;
    SPI_BUS_SPEED spiSpeed50khz;
    SPI_BUS_SPEED spiSpeed31k25hz;
    SPI_BUS_SPEED spiSpeed25khz;
    SPI_BUS_SPEED spiSpeed10khz;
    SPI_BUS_SPEED spiSpeed5khz;
    SPI_BUS_SPEED spiSpeed1khz;

    /// Set one the SPI clock rate.
    BOOL _setClockRate(ULONG clockKhz);

    /// Set the data width shifted on this SPI bus.
    BOOL _setDataWidth(ULONG bits);

    /// Device handle used to map SPI controller registers into user-mode address space.
    HANDLE m_hController;

    /// Pointer SPI controller registers mapped into this process' address space.
    PSPI_CONTROLLER m_controller;

    /// If TRUE invert the data before/after transfer (Controller only supports MSB first).
    BOOL m_flipBitOrder;

    /// The size of the data sent and received on this SPI bus.
    ULONG m_dataBits;

};

/**
Transfer a number of bits on the SPI bus.
\param[in] dataOut A byte of data to send on the SPI bus
\param[out] datIn The byte of data reaceived on the SPI bus
\param[in] bits The number of bits to transfer in each direction on the bus
\return TRUE, success. FALSE, failure, GetLastError() returns the error code.
*/
inline BOOL SPIControllerClass::transfer(ULONG dataOut, ULONG & dataIn, ULONG bits)
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

    if (status && (m_dataBits != bits))
    {
        _setDataWidth(bits);
    }

    if (status)
    {
        txData = dataOut;
        // Flip the bit order if needed.
        if (m_flipBitOrder)
        {
            tmpData = dataOut;
            for (i = 0; i < (bits - 1); i++)
            {
                txData = txData << 1;
                tmpData = tmpData >> 1;
                txData = txData | (tmpData & 0x01);
            }
        }
        txData = txData & (0xFFFFFFFF >> (32 - bits));

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

        tmpData = rxData;
        // Flip the received data bit order if needed.
        if (m_flipBitOrder)
        {
            for (i = 0; i < (bits - 1); i++)
            {
                tmpData = tmpData << 1;
                rxData = rxData >> 1;
                tmpData = tmpData | (rxData & 0x01);
            }
        }
        dataIn = tmpData & (0xFFFFFFFF >> (32 - bits));
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

#endif  // _SPI_CONTROLLER_H_