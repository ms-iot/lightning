/** \file spi.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.
 * See License.txt in the project root for license information.
 */

#ifndef _WINDOWS_SPI_H_
#define _WINDOWS_SPI_H_

#include "arduino.h"

/// \brief Define SPI modes
/// \details Different modes have different clock polarities and/or
/// clock edge for bit shift vs capture.
#define SPI_MODE0    0  ///< Clock sits low, capture on rising edge
#define SPI_MODE1    1  ///< Clock sits low, shift on rising edge
#define SPI_MODE2    2  ///< Clock sits high, capture on falling edge
#define SPI_MODE3    3  ///< Clock sits high, shift on falling edge

// Define the Arduino pin numbers for SPI functions.
#define SPI1_MOSI 11  ///< (M)aster (O)ut (S)lave (I)n
#define SPI1_MISO 12  ///< (M)aster (I)n (S)lave (O)ut
#define SPI1_SCK 13  ///< (S)erial (C)loc(K)

// Define the externally accessible SPI bus characteristics.
#define SPI1_SPEED 1000000UL  // Default speed is 1 Mhz
#define SPI1_BITS_PER_WORD 8  // Send/receive 8 bit words
#define SPI1_MODE SPI_MODE0   // Clock sits low between words,
                              //  capture data on rising edge

// Define SPI clock divider values.
#define SPI_CLOCK_DIV_MIN 1
#define SPI_CLOCK_DIV2 1
#define SPI_CLOCK_DIV4 2
#define SPI_CLOCK_DIV8 3
#define SPI_CLOCK_DIV16 4
#define SPI_CLOCK_DIV32 5
#define SPI_CLOCK_DIV64 6
#define SPI_CLOCK_DIV128 7
#define SPI_CLOCK_DIV_MAX 7
#define SPI_CLOCK_DIV_DEFAULT 2

// Bit order values.
#define LSBFIRST        0x00
#define MSBFIRST        0x01

/// \brief Serial Peripheral Interface communication class
/// \details Serial Peripheral Interface (SPI) is a synchronous
/// serial data protocol used by microcontrollers for communicating
/// with one or more peripheral devices quickly over short distances.
/// It can also be used for communication between two microcontrollers.
/// \see <a href="http://arduino.cc/en/Reference/SPI" target="_blank">origin: Arduino::SPI</a>
class SPIClass
{
public:
	SPIClass() : spi(nullptr) { }

	virtual ~SPIClass()
	{
		this->end();
	}

    /// \brief Initializes the SPI bus
    /// \details Sets SCK, MOSI, and SS to outputs, pulling SCK
    /// and MOSI low, and SS high.
    /// \see SPI::end
    /// \see SPI::setClockDivider
    /// \see SPI::setDataMode
    /// \see SPI::setBitOrder
    /// \see <a href="http://arduino.cc/en/Reference/SPIBegin" target="_blank">origin: Arduino::SPI::begin</a>
    void begin()
	{
		SPI_CONTROLLER_CONFIG cfg;

		if (this->spi == nullptr)
		{
			_ValidatePinOkToChange(SPI1_MOSI);
			_ValidatePinOkToChange(SPI1_MISO);
			_ValidatePinOkToChange(SPI1_SCK);

			HRESULT hr = SpiCreateInstance(SPI_CONTROLLER_INDEX, &this->spi);
			if (FAILED(hr))
			{
				ThrowError("Failed to initialize SPI_CONTROLLER");
			}

			cfg.ConnectionSpeed = SPI1_SPEED;
			cfg.DataBitLength = SPI1_BITS_PER_WORD;
			cfg.SpiMode = SPI1_MODE;
            _bitOrder = MSBFIRST;

			hr = SpiSetControllerConfig(this->spi, &cfg);
			if (FAILED(hr))
			{
				ThrowError("Failed to set controller configuration");
			}

			// Set up pin for SPI MOSI use.
			_SetImplicitPinMode(SPI1_MOSI, OUTPUT);
			_SetImplicitPinFunction(SPI1_MOSI, ALTERNATE_MUX);
			_pinData[SPI1_MOSI].pinInUseSpi = TRUE;
			_pinData[SPI1_MOSI].pinIsLocked = TRUE;

			// Set up pin for SPI MISO use.
			_SetImplicitPinMode(SPI1_MISO, INPUT);
			_SetImplicitPinFunction(SPI1_MISO, ALTERNATE_MUX);
			_pinData[SPI1_MISO].pinInUseSpi = TRUE;
			_pinData[SPI1_MISO].pinIsLocked = TRUE;

			// Set up pin for SPI Clock use.
			_SetImplicitPinMode(SPI1_SCK, OUTPUT);
			_SetImplicitPinFunction(SPI1_SCK, ALTERNATE_MUX);
			_pinData[SPI1_SCK].pinInUseSpi = TRUE;
			_pinData[SPI1_SCK].pinIsLocked = TRUE;

		}
    }

    /// \brief Disables the SPI bus (leaving pin modes unchanged)
    /// \see SPI::begin
    /// \see <a href="http://arduino.cc/en/Reference/SPIEnd" target="_blank">origin: Arduino::SPI::end</a>
    void end()
	{
		if (this->spi != nullptr)
		{
			SpiFree(this->spi);
			this->spi = nullptr;

			_pinData[SPI1_MOSI].pinInUseSpi = FALSE;
			_pinData[SPI1_MOSI].pinIsLocked = FALSE;
			_RevertImplicitPinFunction(SPI1_MOSI);
			pinMode(SPI1_MOSI, OUTPUT);

			_pinData[SPI1_MISO].pinInUseSpi = FALSE;
			_pinData[SPI1_MISO].pinIsLocked = FALSE;
			_RevertImplicitPinFunction(SPI1_MISO);
			pinMode(SPI1_MISO, INPUT);

			_pinData[SPI1_SCK].pinInUseSpi = FALSE;
			_pinData[SPI1_SCK].pinIsLocked = FALSE;
			_RevertImplicitPinFunction(SPI1_SCK);
			pinMode(SPI1_SCK, OUTPUT);
		}
	}

	/// \brief Sets the SPI data mode: that is, clock polarity and phase
    /// \param [in] mode
    /// \n Valid values:
    /// \arg SPI_MODE0 -> CLKP:0 PH:0
    /// \arg SPI_MODE1 -> CLKP:0 PH:1
    /// \arg SPI_MODE2 -> CLKP:1 PH:0
    /// \arg SPI_MODE3 -> CLKP:1 PH:1
    /// \see SPI::setClockDivider
    /// \see SPI::setBitOrder
    /// \see <a href="http://arduino.cc/en/Reference/SPISetDataMode" target="_blank">origin: Arduino::SPI::setDataMode</a>
    void setDataMode(int mode)
	{
		SPI_CONTROLLER_CONFIG cfg;

		HRESULT hr = SpiGetControllerConfig(this->spi, &cfg);
		if (FAILED(hr))
		{
			ThrowError("Failed to get SPI_CONTROLLER controller configuration");
		}

		cfg.SpiMode = mode;
		hr = SpiSetControllerConfig(this->spi, &cfg);
		if (FAILED(hr))
		{
			ThrowError("Failed to set SPI_CONTROLLER controller configuration");
		}
	}

    /// \brief Sets the SPI clock divider relative to the system clock
    /// \param [in] divider
    /// \n Valid values:
    /// \arg SPI_CLOCK_DIV2
    /// \arg SPI_CLOCK_DIV4
    /// \arg SPI_CLOCK_DIV8
    /// \arg SPI_CLOCK_DIV16
    /// \arg SPI_CLOCK_DIV32
    /// \arg SPI_CLOCK_DIV64
    /// \arg SPI_CLOCK_DIV128
    /// \see SPI::setDataMode
    /// \see SPI::setBitOrder
    /// \see <a href="http://arduino.cc/en/Reference/SPISetClockDivider" target="_blank">origin: Arduino::SPI::setClockDivider</a>
    void setClockDivider(int clockDiv)
	{
		ULONG div = 0;
		SPI_CONTROLLER_CONFIG cfg;

		if ((clockDiv < SPI_CLOCK_DIV_MIN) || (clockDiv > SPI_CLOCK_DIV_MAX))
		{
			div = 1UL << SPI_CLOCK_DIV_DEFAULT;
		}
		else
		{
			div = 1UL << clockDiv;
		}

		HRESULT hr = SpiGetControllerConfig(this->spi, &cfg);
		if (FAILED(hr))
		{
			ThrowError("Failed to get SPI_CONTROLLER controller configuration");
		}

		// Arduino UNO clock speed is 16Mhz
		cfg.ConnectionSpeed = ARDUINO_CLOCK_SPEED / div;

		hr = SpiSetControllerConfig(this->spi, &cfg);
		if (FAILED(hr))
		{
			ThrowError("Failed to set SPI_CONTROLLER controller configuration");
		}
	}
    
    // Reverse the bit order or a byte.
	unsigned char _reverseBitOrder(unsigned char valIn)
	{
		unsigned char retVal = 0;

		for (int i = 0; i < 8; i++)
		{
			retVal = retVal << 1;
			retVal = retVal | ((valIn >> i) & 1);
		}
		return retVal;
	}

    /// \brief Transfers one byte over the SPI bus, both sending and receiving.
    /// \param [in] val The byte to send out over the bus
    /// \return The byte read from the bus
    /// \see <a href="http://arduino.cc/en/Reference/SPITransfer" target="_blank">origin: Arduino::SPI::transfer</a>
    unsigned char transfer(unsigned char val)
	{
        unsigned char ret;
        unsigned char writeVal;
        
        if(_bitOrder == LSBFIRST)
        {
            writeVal = _reverseBitOrder(val);
        }
        else
        {
            writeVal = val;
        }
        
		HRESULT hr = SpiTransfer(this->spi, 0, &writeVal, sizeof(writeVal), &ret, sizeof(ret));
		if (FAILED(hr))
		{
			ThrowError("SPI_CONTROLLER transfer failed");
		}

        if(_bitOrder == LSBFIRST)
        {
            ret = _reverseBitOrder(ret);
        }
        
		return ret;
	}

	SPI_CONTROLLER *handle() const
	{
		return spi;
	}
    
    /// \brief Sets the order of the bits shifted out of and into the SPI bus
	/// \details either LSBFIRST (least-significant bit first) or MSBFIRST (most-significant bit first)
    /// \param [in] order LSBFIRST or MSBFIRST
    /// \note MSBFIRST, unless specified otherwise
    /// \see SPI::setClockDivider
    /// \see SPI::setDataMode
    /// \see <a href="http://arduino.cc/en/Reference/SPISetBitOrder" target="_blank">origin: Arduino::SPI::setBitOrder</a>
    void setBitOrder(int bitOrder)
	{
		if (bitOrder == LSBFIRST)
		{
			_bitOrder = LSBFIRST;
		}
		else
		{
			_bitOrder = MSBFIRST;
		}
	}

private:
	SPI_CONTROLLER *spi;
    int _bitOrder;
};

__declspec(selectany) SPIClass SPI;

#endif
