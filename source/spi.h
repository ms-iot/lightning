#ifndef _WINDOWS_SPI_H_
#define _WINDOWS_SPI_H_

#include "arduino.h"

// Define SPI modes.  Different modes have different clock
// polarities and/or clock edge for bit shift vs capture.
#define SPI_MODE0    0  // Clock sits low, capture on rising edge
#define SPI_MODE1    1  // Clock sits low, shift on rising edge
#define SPI_MODE2    2  // Clock sites high, capture on falling edge
#define SPI_MODE3    3  // Clock sits high, shift on falling edge

// Define the Arduino pin numbers for SPI functions.
#define SPI1_MOSI 11
#define SPI1_MISO 12
#define SPI1_SCK 13

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

class SPIClass
{
public:
	SPIClass() : spi(nullptr) { }

	virtual ~SPIClass()
	{
		this->end();
	}

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

	// sets the specified mode, assuming 8-bit data length
	// mode should be SPI_MODE0 ... SPI_MODE3
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

	// Transfer one byte over the SPI in each direction (send one, receive one).
	unsigned char transfer(unsigned char val)
	{
		unsigned char ret;
		HRESULT hr = SpiTransfer(this->spi, 0, &val, sizeof(val), &ret, sizeof(ret));
		if (FAILED(hr))
		{
			ThrowError("SPI_CONTROLLER transfer failed");
		}

		return ret;
	}

	SPI_CONTROLLER *handle() const
	{
		return spi;
	}

private:
	SPI_CONTROLLER *spi;
};

__declspec(selectany) SPIClass SPI;

#endif
