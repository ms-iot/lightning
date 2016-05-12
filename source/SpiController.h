// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _SPI_CONTROLLER_H_
#define _SPI_CONTROLLER_H_

#include <Windows.h>
#include "DmapSupport.h"
#include "BoardPins.h"

#define ADC_SPI_BUS 0
#define EXTERNAL_SPI_BUS 1
#define SECOND_EXTERNAL_SPI_BUS 2

#define DEFAULT_SPI_CLOCK_KHZ 4000
#define DEFAULT_SPI_MODE 0
#define DEFAULT_SPI_BITS 8

class SpiControllerClass
{
public:
    /// Constructor.
    SpiControllerClass() :
        m_flipBitOrder(FALSE),
        m_dataBits(DEFAULT_SPI_BITS),
        m_sckPin(0xFFFFFFFF),
        m_mosiPin(0xFFFFFFFF),
        m_misoPin(0xFFFFFFFF)
    {
    }

    /// Destructor.
    virtual ~SpiControllerClass()
    {
    }

    /// Initialize the pin assignments for this SPI controller.
    virtual HRESULT configurePins(ULONG misoPin, ULONG mosiPin, ULONG sckPin) = 0;

    /// Revert the pins used by this SPI controller to GPIO use.
    LIGHTNING_DLL_API HRESULT revertPinsToGpio();

    /// Initialize the specified SPI bus, using the default mode and clock for the controller.
    /**
    \param[in] busNumber The number of the SPI bus to open (0 - 2)
    \return HRESULT success or error code.
    */
    virtual HRESULT begin(ULONG busNumber) = 0;

    /// Initialize the specified SPI bus for use.
    virtual HRESULT begin(ULONG busNumber, ULONG mode, ULONG clockKhz, ULONG dataBits) = 0;

    /// Finish using the SPI controller associated with this object.
    virtual void end() = 0;

    /// Method to set the default bit order: MSB First.
    LIGHTNING_DLL_API void setMsbFirstBitOrder();

    /// Method to set the alternate bit order: LSB First.
    LIGHTNING_DLL_API void setLsbFirstBitOrder();

    /// Set the SPI clock rate.
    virtual HRESULT setClock(ULONG clockKhz) = 0;

    /// Set the SPI mode (clock polarity and phase).
    virtual HRESULT setMode(ULONG mode) = 0;

    /// Set the number of bits in an SPI transfer.
    virtual HRESULT setDataWidth(ULONG bits) = 0;

    /// Transfer a byte of data on the SPI bus.
    /**
    \param[in] dataOut A byte of data to send on the SPI bus
    \param[out] datIn The byte of data received on the SPI bus
    \return HRESULT success or error code.
    */
    LIGHTNING_DLL_API HRESULT transfer8(ULONG dataOut, ULONG & dataIn);

    /// Transfer a word of data on the SPI bus.
    /**
    \param[in] dataOut A word of data to send on the SPI bus
    \param[out] datIn The word of data received on the SPI bus
    \return HRESULT success or error code.
    */
    LIGHTNING_DLL_API HRESULT transfer16(ULONG dataOut, ULONG & dataIn);

    /// Transfer three bytes of data on the SPI bus.
    /**
    \param[in] dataOut Three bytes of data to send on the SPI bus
    \param[out] datIn The three bytes of data received on the SPI bus
    \return HRESULT success or error code.
    */
    LIGHTNING_DLL_API HRESULT transfer24(ULONG dataOut, ULONG & dataIn);

    /// Transfer a longword of data on the SPI bus.
    /**
    \param[in] dataOut A longword of data to send on the SPI bus
    \param[out] datIn The longword of data received on the SPI bus
    \return HRESULT success or error code.
    */
    LIGHTNING_DLL_API HRESULT transfer32(ULONG dataOut, ULONG & dataIn);

    /// Perform a non-bytesized transfer on the SPI bus.
    /**
    \param[in] dataOut The data to send on the SPI bus
    \param[out] datIn The data received on the SPI bus
    \param[in] bits The number of bits to transfer
    \return HRESULT success or error code.
    */
    LIGHTNING_DLL_API HRESULT transferN(ULONG dataOut, ULONG & dataIn, ULONG bits);

    /// Transfer a buffer of data on the SPI bus.
    /**
    \param[in] dataOut A pointer to a buffer of data to send on the SPI bus
    \param[in] datIn A pointer to a buffer to receive from the SPI bus
    \param[in] bufferBytes The number of bytes to transfer.  Each bufffer 
    must be at least this long.
    \return HRESULT success or error code.
    \note The data is sent and received MSbit first.  If LSbit first transfer, or 
    any other special ordering of the bytes in the buffer, is needed, the data must
    be organized appropriately in the buffer before it is handed to this method.
    */
    virtual inline HRESULT transferBuffer(PBYTE dataOut, PBYTE dataIn, size_t bufferBytes) = 0;

protected:
    /// SPI Clock pin number.
    ULONG m_sckPin;

    /// SPI Master Out Slave In pin number.
    ULONG m_mosiPin;

    /// SPI Master In Slave Out pin number.
    ULONG  m_misoPin;

    /// The number of bits in an SPI transfer.
    ULONG m_dataBits;

private:

    /// If TRUE invert the data before/after transfer (Controller only supports MSB first).
    BOOL m_flipBitOrder;

    /// Array of values use to flip the bit order of a byte.
    static const UCHAR m_byteFlips[256];

    /// Method to perform a transfer on the SPI controller on this board.
    /**
    \param[in] dataOut The data to send on the SPI bus
    \param[out] datIn The data received on the SPI bus
    \param[in] bits The number of bits to transfer
    \return HRESULT success or error code.
    */
    virtual inline HRESULT _transfer(ULONG dataOut, ULONG & dataIn, ULONG bits) = 0;
};

#endif  // _SPI_CONTROLLER_H_