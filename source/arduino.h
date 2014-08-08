// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _WINDOWS_ARDUINO_H_
#define _WINDOWS_ARDUINO_H_

// Arduino compatibility header for inclusion by user programs

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef USE_NETWORKSERIAL
#include <winsock2.h>
#include <WS2tcpip.h>
#endif
#include <cstdint>

#define _USE_MATH_DEFINES
#include <math.h>

#include "ArduinoError.h"
#include "WindowsRandom.h"
#include "WindowsTime.h"
#include "WString.h"
#include "embprpusr.h"
#include "galileo.h"
#include "binary.h"
#include "wire.h"

#define NUM_ARDUINO_PINS 20
#define NUM_ANALOG_PINS 6

#define GALILEO_A0      14

#define NOT_MUXED       0x0
#define DEFAULT_MUX     0x0
#define ALTERNATE_MUX   0x1

#define ARDUINO_CLOCK_SPEED 16000000UL    // 16 Mhz

#define CY8_ADDRESS			0x20	// I2C address of CY8C9540A I/O Expander
#define INPUT_PORTS_BASE	0x00	// Address of first input port register
#define OUTPUT_PORTS_BASE	0x08	// Address of first output port register
#define CY8_PORT_SELECT		0x18	// Address of Port Select register in I/O Expander
#define CY8_PORT_CONFIG_BASE 0x19	// Address of first port configuration register in IOX
#define CY8_PWM_SELECT		0x28	// Address of PWM Select register in I/O Expander
#define CY8_PWM_CONFIG_BASE 0x29	// Address of first PWM configuration register in IOX

#define I2C_MUX GPORT1_BIT5			// Encoded port and bit for I2C MUX
#define I2C_MUX_DISABLE 1			// MUX pin state to disable I2C through MUX
#define IO18_A4_MUX GPORT0_BIT5		// Encoded port and bit for IO18/A4 MUX
#define IO19_A5_MUX GPORT0_BIT4		// Encoded port and bit for IO19/A5 MUX
#define IO_A_MUX_TO_IO 1			// IOnn/An MUX bit state for IO through MUX

#define A0 14						// Analog pin 0 is digital "pin 14"
#define A1 15						// Analog pin 1 is digital "pin 15"
#define A2 16						// Analog pin 2 is digital "pin 16"
#define A3 17						// Analog pin 3 is digital "pin 17"
#define A4 18						// Analog pin 4 is digital "pin 18"
#define A5 19						// Analog pin 5 is digital "pin 19"

// Closest Galileo value to the 490hz used by most UNO PWMs, while still 
// allowing full 8-bit PWM pulse width resolution, is 367 hz.
#define PWM_HZ 367

//
// Definition of Constants as defined on http://wiring.org.co/reference/index.html and http://arduino.cc/en/Reference/Constants
//
#define LOW             0x00
#define HIGH            0x01

#define INPUT           0x00
#define OUTPUT          0x01
#define INPUT_PULLUP    0x02

#define LSBFIRST        0x00
#define MSBFIRST        0x01

#define CHANGE          0x01
#define FALLING         0x02
#define RISING          0x03

#define WLED            (QRK_LEGACY_RESUME_SUS1)
#define LED_BUILTIN     WLED

#define PI              3.1415926535897932384626433832795
#define HALF_PI         1.5707963267948966192313216916398
#define TWO_PI          6.283185307179586476925286766559

#define boolean bool
typedef unsigned char byte;

typedef unsigned short word;

//
// Printf like function to log to stdout and if a debugger is attached to the debugger output.
//
inline int Log(const wchar_t *format, ...)
{
    va_list args;
    int len = 0;
    wchar_t* buffer;

    va_start(args, format);
    len = _vscwprintf(format, args) + 1;
    buffer = (wchar_t*)malloc(len*sizeof(wchar_t));
    if (buffer != NULL)
    {
        len = vswprintf_s(buffer, len, format, args);
        wprintf(buffer);
        if (IsDebuggerPresent())
        {
            OutputDebugStringW(buffer);
        }
        free(buffer);
    }
    else
    {
        len = 0;
    }
    return len;
}

// Arduino math definitions
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define sq(x) ((x)*(x))
inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function prototypes.
inline void pinMode(unsigned int pin, unsigned int mode);

// Internal functions (call at your own risk)
inline void _RevertPinToDigital(int pin);
inline bool _PinFunction(ULONG pin, ULONG function);
inline void _InitializePin(int pin);

// Type of struct used to store state and configuration information
// for a GPIO pin.  Storing this information allows pins to be configured 
// with a minimum number of transfers to the I/O Expander and also allows 
// pin configurations to be restored when needed after changes have been 
// made for an alternate pin function (PWM vs GPIO use, for example).
typedef struct {   // Comment format: Initialized value - Description
    UINT pwmDutyCycle : 8;      // 0 - Pulse width (0 = 0%, 255 = 100%)
    UINT currentMode : 1;       // INPUT - INPUT or OUTPUT
    UINT modeSet : 1;           // INPUT - INPUT or OUTPUT set explicitely
    UINT currentMux : 1;        // DEFAULT_MUX - DEFAULT_MUX or ALTERNATE_MUX
    UINT muxSet : 1;            // DEFAULT_MUX - DEFAULT_MUX or ALTERNATE_MUX set explicitely
    UINT state : 1;             // LOW - LOW or HIGH
    BOOL stateIsKnown : 1;      // FALSE - TRUE or FALSE
    BOOL pwmIsEnabled : 1;      // FALSE - TRUE or FALSE
    BOOL pinIsLocked : 1;       // FALSE - TRUE or FALSE
    BOOL pinInUseSpi : 1;       // FALSE - TRUE or FALSE
    BOOL pinInUseI2c : 1;		// FALSE - TRUE or FALSE
    BOOL pinInitialized : 1;	// FALSE - TRUE or FALSE
} PIN_DATA, *PPIN_DATA;

//
// Pauses the program for the amount of time (in microseconds) 
// specified as parameter.
//
inline void delayMicroseconds(unsigned int us)
{
    LARGE_INTEGER us64;
    us64.QuadPart = us;
    _WindowsTime.delayMicroseconds(us64);
}

//
// Pauses the program for the amount of time (in miliseconds) 
// specified as parameter.
//
inline void delay(unsigned long ms)
{
    _WindowsTime.delay(ms);
}

// Returns the number of milliseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 50 days.
inline unsigned long millis(void)
{
    return _WindowsTime.millis();
}

// Returns the number of microseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 70 minutes.
inline unsigned long micros(void)
{
    return _WindowsTime.micros();
}

//
// Returns true if an Arduino pin number is also an analog input
//
inline bool _IsAnalogPin(int num)
{
    return num >= GALILEO_A0;
}

// set up all the muxes
// index in the map corresponds to Arduino pin number
const int _GalileoMuxMap[NUM_ARDUINO_PINS] = {
    // lo word is the GPIO pin that the mux is connected to,
    // hi word is the state (0 or 1) that the mux should
    // be in to select this GPIO
    // first is lo word, 2nd is high word
    MAKELONG(GPORT3_BIT4, ALTERNATE_MUX),           // IO0_MUX
    MAKELONG(GPORT3_BIT5, ALTERNATE_MUX),           // IO1_MUX
    MAKELONG(GPORT1_BIT7, DEFAULT_MUX),             // IO2_MUX
    MAKELONG(GPORT1_BIT6, DEFAULT_MUX),             // IO3_MUX
    NOT_MUXED,                                      // IO4 not muxed
    NOT_MUXED,                                      // IO5 not muxed
    NOT_MUXED,                                      // IO6 not muxed
    NOT_MUXED,                                      // IO7 not muxed
    NOT_MUXED,                                      // IO8 not muxed
    NOT_MUXED,                                      // IO9 not muxed
    MAKELONG(GPORT3_BIT6, DEFAULT_MUX),             // IO10_MUX
    MAKELONG(GPORT3_BIT7, ALTERNATE_MUX),           // IO11_MUX
    MAKELONG(GPORT5_BIT2, ALTERNATE_MUX),           // IO12_MUX
    MAKELONG(GPORT5_BIT3, ALTERNATE_MUX),           // IO13_MUX
    MAKELONG(GPORT3_BIT1, DEFAULT_MUX),             // IO14 - A0_MUX: 0=Analog, 1=GPIO
    MAKELONG(GPORT3_BIT0, DEFAULT_MUX),             // IO15 - A1_MUX: 0=Analog, 1=GPIO
    MAKELONG(GPORT0_BIT7, DEFAULT_MUX),             // IO16 - A2_MUX: 0=Analog, 1=GPIO
    MAKELONG(GPORT0_BIT6, DEFAULT_MUX),             // IO17 - A3_MUX: 0=Analog, 1=GPIO
    MAKELONG(GPORT0_BIT5, DEFAULT_MUX),             // IO18 - A4_MUX: 0=Analog, 1=GPIO
    MAKELONG(GPORT0_BIT4, DEFAULT_MUX)              // IO19 - A5_MUX: 0=Analaog, 1=GPIO
};


const ULONG _ArduinoToGalileoPinMap[NUM_ARDUINO_PINS] =
{
    GPORT4_BIT6,                // 0
    GPORT4_BIT7,                // 1
    QRK_IOFABRIC_GPIO_6,        // 2
    QRK_IOFABRIC_GPIO_7,        // 3
    GPORT1_BIT4,                // 4
    GPORT0_BIT1_PWM5,           // 5
    GPORT1_BIT0_PWM6,           // 6
    GPORT1_BIT3_PWM0,           // 7
    GPORT1_BIT2_PWM2,           // 8
    GPORT0_BIT3_PWM1,           // 9
    QRK_IOFABRIC_GPIO_2,        // 10
    GPORT1_BIT1_PWM4,           // 11
    GPORT3_BIT2,                // 12
    GPORT3_BIT3,                // 13
    GPORT4_BIT0,                // 14 - A0
    GPORT4_BIT1,                // 15 - A1
    GPORT4_BIT2,                // 16 - A2
    GPORT4_BIT3,                // 17 - A3
    GPORT4_BIT4,                // 18 - A4
    GPORT4_BIT5,                // 19 - A5
};

// This table is used to look up the port bit of an Arduino pin.
typedef struct {
    BYTE PortNumber;
    BYTE BitNumber;
    BYTE PwmNumber;
    BYTE MuxPort;
    BYTE MuxBit;
    BYTE IsSpiPin;
    BYTE IsI2cPin;
    BYTE IsAnalogPin;
} PORT_BIT, *PPORT_BIT;
#define NO_PORT 0x0F
#define NO_PWM 0x0F
const PORT_BIT _ArduinoToPortBitMap[NUM_ARDUINO_PINS] =
{  // Format: port, port-bit, PWM-number, MUX-port, MUX-port-bit, SPI-pin, I2C-pin, analog-pin
    { 4, 6, NO_PWM, 3, 4, false, false, false },			// 0
    { 4, 7, NO_PWM, 3, 5, false, false, false },			// 1
    { NO_PORT, 0, NO_PWM, 1, 7, false, false, false },		// 2
    { NO_PORT, 0, 3, 1, 6, false, false, false },			// 3
    { 1, 4, NO_PWM, NO_PORT, 0, false, false, false },		// 4
    { 0, 1, 5, NO_PORT, 0, false, false, false },			// 5
    { 1, 0, 6, NO_PORT, 0, false, false, false },			// 6
    { 1, 3, 0, NO_PORT, 0, false, false, false },			// 7
    { 1, 2, 2, NO_PORT, 0, false, false, false },			// 8
    { 0, 3, 1, NO_PORT, 0, false, false, false },			// 9
    { NO_PORT, 0, 7, 3, 6, false, false, false },			// 10
    { 1, 1, 4, 3, 7, true, false, false },					// 11
    { 3, 2, NO_PWM, 5, 2, true, false, false },				// 12
    { 3, 3, NO_PWM, 5, 3, true, false, false },				// 13
    { 4, 0, NO_PWM, 3, 1, false, false, true },				// 14 - A0
    { 4, 1, NO_PWM, 3, 0, false, false, true },				// 15 - A1
    { 4, 2, NO_PWM, 0, 7, false, false, true },				// 16 - A2
    { 4, 3, NO_PWM, 0, 6, false, false, true },				// 17 - A3
    { 4, 4, NO_PWM, 0, 5, false, true, true },				// 18 - A4
    { 4, 5, NO_PWM, 0, 4, false, true, true }				// 19 - A5
};

// Offsets from port configuration register base to each register.
typedef enum _PORT_CONFIG_REGS {
    INT_MASK = 0,
    SELECT_PWM,
    INVERSION,
    PIN_DIRECTION,
    DRIVE_PULL_UP,
    DRIVE_PULL_DOWN,
    DRIVE_OPEN_DRAIN_HIGH,
    DRIVE_OPEN_DRAIN_LOW,
    DRIVE_STRONG,
    DRIVE_SLOW_STRONG,
    DRIVE_HIGH_Z,
    PORT_CONFIG_REG_COUNT
} PORT_CONFIG_REGS;

// Offsets from PWM configuration register base to each register.
typedef enum _PWM_CONFIG_REGS {
    CONFIG_PWM = 0,
    PERIOD_PWM,
    PULSE_WIDTH_PWM,
    PWM_CONFIG_REG_COUNT
} PWM_CONFIG_REGS;

// This function throws an error if the specified pin number is not a valid
// Arduino GPIO pin number.
inline void _ValidateArduinoPinNumber(int pin)
{
    if ( (pin < 0) || (pin >= NUM_ARDUINO_PINS) )
    {
        ThrowError("Invalid pin number (%d). Pin must be in the range [0, %d)",
            pin, NUM_ARDUINO_PINS);
    }
}

// This table specifies the mux function for PWM use on a pin.
// The table is indexed by Arduino GPIO pin number.
const UCHAR _PwmMuxFunction[NUM_ARDUINO_PINS]
{
    DEFAULT_MUX,				// Pin 0 - No PWM
    DEFAULT_MUX,				// Pin 1 - No PWM
    DEFAULT_MUX,				// Pin 2 - No PWM
    ALTERNATE_MUX,				// Pin 3 - PWM uses alternate MUX
    DEFAULT_MUX,				// Pin 4 - No PWM, no MUX
    DEFAULT_MUX,				// Pin 5 - PWM, no MUX
    DEFAULT_MUX,				// Pin 6 - PWM, no MUX
    DEFAULT_MUX,				// Pin 7 - PWM, no MUX
    DEFAULT_MUX,				// Pin 8 - PWM, no MUX
    DEFAULT_MUX,				// Pin 9 - PWM, no MUX
    ALTERNATE_MUX,				// Pin 10 - PWM uses alternate MUX
    DEFAULT_MUX,			    // Pin 11 - PWM uses default MUX
    DEFAULT_MUX,			    // Pin 12 - PWM uses default MUX
    DEFAULT_MUX,			    // Pin 13 - PWM uses default MUX
    DEFAULT_MUX,			    // Pin 14 - No PWM
    DEFAULT_MUX,			    // Pin 15 - No PWM
    DEFAULT_MUX,			    // Pin 16 - No PWM
    DEFAULT_MUX,			    // Pin 17 - No PWM
    DEFAULT_MUX,			    // Pin 18 - No PWM
    DEFAULT_MUX				    // Pin 19 - No PWM
};

// This table maps PWM pin numbers to I/O Expander pins.
const UCHAR _PwmPinMap[NUM_ARDUINO_PINS]
{
    0,							// Pin 0 - No PWM
    0,							// Pin 1 - No PWM
    0,							// Pin 2 - No PWM
    GPORT0_BIT2_PWM3,			// Pin 3 - PWM 3
    0,							// Pin 4 - No PWM
    GPORT0_BIT1_PWM5,			// Pin 5 - PWM 5
    GPORT1_BIT0_PWM6,			// Pin 6 - PWM 6
    GPORT1_BIT3_PWM0,			// Pin 7 - PWM 0
    GPORT1_BIT2_PWM2,			// Pin 8 - PWM 2
    GPORT0_BIT3_PWM1,			// Pin 9 - PWM 1
    GPORT0_BIT0_PWM7,			// Pin 10 - PWM 7
    GPORT1_BIT1_PWM4,			// Pin 11 - PWM 4
    0,							// Pin 12 - No PWM
    0,							// Pin 13 - No PWM
    0,							// Pin 14 - No PWM
    0,							// Pin 15 - No PWM
    0,							// Pin 16 - No PWM
    0,							// Pin 17 - No PWM
    0,							// Pin 18 - No PWM
    0							// Pin 19 - No PWM
};

// This array stores state and configuration information for each GPIO pin.
__declspec (selectany) PIN_DATA _pinData[NUM_ARDUINO_PINS] = { 0 };

// Read the current configuration of a pin from the I/O Expander.
// This function assumes the caller has verified the pin number for range
// and has initialized the PIN_DATA struct passed in to zeroes.
//
// The "inline" marks the function as "selectany", while "__declspec (noinline)"
// keeps it from actually being expanded inline.  Without the "inline" the linker
// will complain about multiple definitions of the same routine name.
__declspec (noinline) inline void _ReadPinConfiguration(int pin, PPIN_DATA pinData)
{
    int i;
    BYTE portConfig[PORT_CONFIG_REG_COUNT] = { 0 };
    BYTE pwmConfig[PWM_CONFIG_REG_COUNT] = { 0 };
    BYTE retVal = 0;

    // Indicate which port we want the configuration of.
    Wire.beginTransmission(CY8_ADDRESS);
    Wire.write(CY8_PORT_SELECT);
    Wire.write(_ArduinoToPortBitMap[pin].PortNumber);
    Wire.endTransmission(TRUE);							// Send STOP

    // Send the base address of the configuration registers.
    Wire.beginTransmission(CY8_ADDRESS);
    Wire.write(CY8_PORT_CONFIG_BASE);
    Wire.endTransmission(FALSE);						// Dont' send STOP

    // Read the configuration registers.
    retVal = static_cast<BYTE>(Wire.requestFrom(CY8_ADDRESS, PORT_CONFIG_REG_COUNT));
    if (retVal != PORT_CONFIG_REG_COUNT)
    {
        ThrowError("Error reported by Wire.requestFrom() for pin (%d).", pin);
    }
    for (i = 0; i < PORT_CONFIG_REG_COUNT; i++)
    {
        portConfig[i] = static_cast<BYTE>(Wire.read());
    }

    // Determine whether the port pin is configured as an input or output.
    // The I/O Expander uses 0 for output, 1 for input.
    if ((portConfig[PIN_DIRECTION] & (1 << _ArduinoToPortBitMap[pin].BitNumber)) == 0)
    {
        pinData->currentMode = OUTPUT;
        pinData->modeSet = OUTPUT;
    }
    else
    {
        pinData->currentMode = INPUT;
        pinData->modeSet = INPUT;
    }

    // If the port pin is configured as a PMW:
    if ((portConfig[SELECT_PWM] & (1 << _ArduinoToPortBitMap[pin].BitNumber)) != 0)
    {
        // If the pin is one on which we support PWM use:
        if (_PwmPinMap[pin] != 0)
        {
            pinData->pwmIsEnabled = TRUE;

            // Indicate which PWM we want the configuration for.
            Wire.beginTransmission(CY8_ADDRESS);
            Wire.write(CY8_PWM_SELECT);
            Wire.write(_ArduinoToPortBitMap[pin].PwmNumber);
            Wire.endTransmission(TRUE);						// Send STOP

            // Send the base address of the PWM configuration registers.
            Wire.beginTransmission(CY8_ADDRESS);
            Wire.write(CY8_PWM_CONFIG_BASE);
            Wire.endTransmission(FALSE);					// Don't send STOP

            // Read the PWM configuration registers.
            retVal = static_cast<BYTE>(Wire.requestFrom(CY8_ADDRESS, PWM_CONFIG_REG_COUNT));
            if (retVal != PWM_CONFIG_REG_COUNT)
            {
                ThrowError("Error reported by Wire.requestFrom() for pin (%d).", pin);
            }
            for (i = 0; i < PWM_CONFIG_REG_COUNT; i++)
            {
                pwmConfig[i] = static_cast<BYTE>(Wire.read());
            }

            pinData->pwmDutyCycle = pwmConfig[PULSE_WIDTH_PWM];
        }
        // If we don't support PWM use on this pin, the user is on his own.
    }
}

__declspec (noinline) inline void _ReadPinMuxConfig(int pin, PPIN_DATA pinData)
{
    unsigned int gpioMux;
    BYTE muxConfig;
    BYTE retVal = 0;

    // If this pin uses a MUX:
    if (_ArduinoToPortBitMap[pin].MuxPort != NO_PORT)
    {
        // Read the output register that controlls the MUX.
        Wire.beginTransmission(CY8_ADDRESS);
        Wire.write(OUTPUT_PORTS_BASE + _ArduinoToPortBitMap[pin].MuxPort);
        Wire.endTransmission(FALSE);					// Don't send STOP

        // Read the output port register.
        retVal = static_cast<BYTE>(Wire.requestFrom(CY8_ADDRESS, 1));
        if (retVal != 1)
        {
            ThrowError("Error reported by Wire.requestFrom() for pin (%d).", pin);
        }
        muxConfig = static_cast<BYTE>(Wire.read());

        // Determine whether the MUX is set to default or alternate value.
        muxConfig = (muxConfig >> _ArduinoToPortBitMap[pin].MuxBit) & 0x01;
        gpioMux = _GalileoMuxMap[pin];
        gpioMux = HIWORD(gpioMux);
        if (muxConfig == gpioMux)
        {
            pinData->currentMux = DEFAULT_MUX;
            pinData->muxSet = DEFAULT_MUX;
        }
        else
        {
            pinData->currentMux = ALTERNATE_MUX;
            pinData->muxSet = ALTERNATE_MUX;
        }
    }
    else
    {
        pinData->currentMux = DEFAULT_MUX;
        pinData->muxSet = DEFAULT_MUX;
    }
}

inline void _InitializePinIfNeeded(int pin)
{
    // If data for the pin in question has not been initialized yet:
    if (!(_pinData[pin].pinInitialized))
    {
        // Initialize the pin.
        _InitializePin(pin);
    }
}

__declspec (noinline) inline void _InitializePin(int pin)
{
    // Get the current state of the I2C MUX.
    bool I2cWasEnabled = Wire.getI2cHasBeenEnabled();
	bool done = false;

    Wire.begin();

    // Restore the state of the I2C MUX if it was changed by the Wire.begin() call.
    if (!I2cWasEnabled)
    {
        GpioWrite(I2C_MUX, I2C_MUX_DISABLE);
        Wire.setI2cHasBeenEnabled(false);
    }

    try
    {
        // If the pin normally comes from the I/O Expander:
        if (_ArduinoToGalileoPinMap[pin] >= GPORT0_BIT0_PWM7)
        {
            // Read the pin configuration from the CY8C9540A I/O Expander.
            _ReadPinConfiguration(pin, &(_pinData[pin]));

            // Indicate the data for this pin is now initialized.
            _pinData[pin].pinInitialized = TRUE;
		
			done = true;
		}

        // Read the MUX configuration for this pin.
        _ReadPinMuxConfig(pin, &(_pinData[pin]));
    }
    catch (const _arduino_fatal_error &)
    {
        done = false;
    }

    // If the pin normally comes directly from the Quark SOC, 
    // or if an error occurred querying the pin configuration:
    if (!done)
    {
        // We can't query the pin configuration, so we just initialize it.
        _pinData[pin].pinInitialized = TRUE;		// Avoid recursive loop calling pinMode()!
        _pinData[pin].currentMode = OUTPUT;			// Force update of pin direction
        pinMode(pin, INPUT);

		// If this is a PWM pin:
		if (_PwmPinMap[pin] != 0)
		{
			// If the MUX is set to alternate:
			if (_pinData[pin].currentMux == ALTERNATE_MUX)
			{
				// Indicate that PWM is enabled on this pin.
				_pinData[pin].pwmIsEnabled = TRUE;
			}
		}
    }

    // If the pin is an SPI pin:
    if (_ArduinoToPortBitMap[pin].IsSpiPin)
    {
        // If the pin MUX is set to alternate, we conclude SPI is in use on this pin.
        if (_pinData[pin].currentMux == ALTERNATE_MUX)
        {
            _pinData[pin].pinInUseSpi = TRUE;
            _pinData[pin].pinIsLocked = TRUE;
        }
    }

    // If the pin is an I2C pin and I2C has been enabled:
    if (_ArduinoToPortBitMap[pin].IsI2cPin && I2cWasEnabled)
    {
        // Indicate the pin is is in use for I2C.
        _pinData[pin].pinInUseI2c = TRUE;
    }

    // If the pin is an analog pin:
    if (_ArduinoToPortBitMap[pin].IsAnalogPin)
    {
        // Set the MUX to the default state (MUX for IO).
        _PinFunction(pin, ALTERNATE_MUX);
    }
}

// Throw an error if the specified pin does not support PWM functions.
inline void _ValidatePwmPin(int pin)
{
    _ValidateArduinoPinNumber(pin);
    if (_PwmPinMap[pin] == 0)
    {
        ThrowError("Invalid PWM pin.  Pin (%d) does not support PWM functions", pin);
    }
}

// Throw an error if a pin in use for SPI is being configured for a conflicting use.
inline void _ValidatePinOkToChange(int pin)
{
    if (_pinData[pin].pinIsLocked)
    {
        ThrowError("Pin %d is already in use (SPI, Serial, etc.) so it can't be reconfigured.", pin);
    }
}

//
// Set the digital pin (IO0 - IO13) to the specified state.
// If the analog pins (A0-A5) are configured as digital IOs,
// also sets the state of these pins.
// A0-A5 are mapped to 14-19
// 
// Examples:
//
//  // set IO4 high.
//  digitalWrite(4, 1);
//  
//  // set A1 low
//  digitalWrite(15, 0);
//
inline void digitalWrite(unsigned int pin, unsigned int state)
{
    _ValidateArduinoPinNumber(pin);

    // Revert the pin if it is in PWM mode.
    _RevertPinToDigital(pin);

    if (state != LOW)
    {
        // Emulate Arduino behavior here. Code like firmata uses bitmasks to set
        // ports, and will pass something like value & 0x20 and expect that to be high.
        state = HIGH;
    }

    if (!_pinData[pin].stateIsKnown || (_pinData[pin].state != state))
    {
        HRESULT hr = GpioWrite(_ArduinoToGalileoPinMap[pin], (ULONG)state);
        if (FAILED(hr))
        {
            ThrowError("GpioWrite() failed. pin=%d, state=%d", pin, state);
        }
        _pinData[pin].state = state;
        _pinData[pin].stateIsKnown = TRUE;
    }
}

//
// Reads the value from the digital pin (IO0 - IO13).
// A0-A5 are mapped to 14-19
//
// Return Value:
//
// 1 for HIGH, 0 for LOW, or -1 for error
// 
// Example:
//
//  // read IO4.
//  int val = digitalRead(4);
//
inline int digitalRead(int pin)
{
    _ValidateArduinoPinNumber(pin);

    // Revert the pin if it is in PWM mode.
    _RevertPinToDigital(pin);
    
    LONG ret;
    HRESULT hr = GpioRead(_ArduinoToGalileoPinMap[pin], &ret);
    if ( FAILED(hr) )
    {
        ThrowError("GpioRead() failed to read pin %d: 0x%x", pin, hr);
    }

    return (int)ret;
}

//
// Configures the specified pin to behave either as an input
// or an output (IO0 - IO13).  A0-A5 are mapped to 14-19
// 
// Example:
//
//  // Set IO4 as input.
//  pinMode(4, INPUT);
//
inline void pinMode(unsigned int pin, unsigned int mode)
{
    _ValidateArduinoPinNumber(pin);
    _ValidatePinOkToChange(pin);
    _InitializePinIfNeeded(pin);

	if (_pinData[pin].pwmIsEnabled)
	{
		PwmStop(_PwmPinMap[pin]);
		_pinData[pin].pwmIsEnabled = FALSE;
	}

    if (_pinData[pin].currentMode != mode)
    {
        HRESULT hr = GpioSetDir(_ArduinoToGalileoPinMap[pin], mode);
        if (FAILED(hr))
        {
            ThrowError("GpioSetDir() failed. pin = %d, mode = %d", pin, mode);
        }
        _pinData[pin].modeSet = mode;
        _pinData[pin].currentMode = mode;
    }
}

//
// Override a pin mode, in such a way that it can be reverted to the last
// explicitely set mode in the future.
//
// This function assumes the pin number has already been verified to be in the
// legal range.
//
inline void _SetImplicitPinMode(unsigned int pin, unsigned int mode)
{
    _InitializePinIfNeeded(pin);

    if (_pinData[pin].currentMode != mode)
    {
        HRESULT hr = GpioSetDir(_ArduinoToGalileoPinMap[pin], mode);
        if (FAILED(hr))
        {
            ThrowError("GpioSetDir() failed. pin = %d, mode = %d", pin, mode);
        }
        _pinData[pin].currentMode = mode;
    }
}

// Revert a pin to its last explicitely set mode.
inline void _RevertImplicitPinMode(unsigned int pin)
{
    pinMode(pin, _pinData[pin].modeSet);
}

// Select a particular function on a multiplexed IO pin in such a way that it 
// can be reverted later if needed.
// This function assumes the pin value has already been checked for range.
inline bool _SetImplicitPinFunction(ULONG pin, ULONG function)
{
    _InitializePinIfNeeded(pin);
    
    unsigned int mux = _GalileoMuxMap[pin];

    if (mux != NOT_MUXED)
    {
        // pin supports alternate configuration. Select default
        // or alternate configuration
        int val = HIWORD(mux);
        switch (function)
        {
        case DEFAULT_MUX:
            // default function
            break;
        case ALTERNATE_MUX:
            // alternate function
            val = !val;
            break;
        default:
            // unsupported function
            return false;
        }

        if (_pinData[pin].currentMux != function)
        {
            _ValidatePinOkToChange(pin);

            HRESULT hr = GpioSetDir(LOWORD(mux), 1);
            if (FAILED(hr))
            {
                ThrowError("GpioSetDir() : Unexpected error");
            }

            hr = GpioWrite(LOWORD(mux), val);
            if (FAILED(hr))
            {
                ThrowError("GpioWrite() : Unexpected error");
            }
            _pinData[pin].currentMux = function;
        }
        return true;
    }
    else
    {
        // pin does not support alternate configurations
        if (function != 0)
            return false;

        // pin is already in default configuration, no change.
        _pinData[pin].currentMux = DEFAULT_MUX;
        return true;
    }
}

//
// Select a particular function on a multiplexed IO pin.
//
// INPUTS:
//   pin - The number of the Arduino GPI pin for which to select
//         the multiplexed function.
//   function - The multiplexer function to select for the pin:
//		   Either DEFAULT_MUX or ALTERNATE_MUX
// RETURN:
//   true - The multiplexer is in the requested state.
//   false - ALTERNATE_MUX was requested on a non-multiplexed pin.
//
// ERRORS:
//   An error is thrown if any of the following occur:
//     - A pin number is specified that is outside the valid range
//     - An attempt is made to change the multiplexer state for a
//       pin that is already locked for an incompatible use
//     - An error is returned from a called routine.
//
inline bool _PinFunction(ULONG pin, ULONG function)
{
    _ValidateArduinoPinNumber(pin);

    if (_SetImplicitPinFunction(pin, function))
    {
        _pinData[pin].muxSet = function;
        return true;
    }
    return false;
}

// Revert a pin to its last explicitely set function.
inline void _RevertImplicitPinFunction(int pin)
{
    _PinFunction(pin, _pinData[pin].muxSet);
}

// Revert the pin to digital I/O use if needed.
// This function assumes the caller has range-checked the pin number.
inline void _RevertPinToDigital(int pin)
{
    _ValidatePinOkToChange(pin);

    if (_pinData[pin].pinInUseI2c)
    {
        GpioWrite(I2C_MUX, I2C_MUX_DISABLE);
        Wire.setI2cHasBeenEnabled(false);
        _pinData[pin].pinInUseI2c = FALSE;
    }
    _RevertImplicitPinFunction(pin);
    _RevertImplicitPinMode(pin);
}

//
// Perform an analog write (PWM) operation.
//
// INPUTS:
//	pin - The Arduino GPIO pin on which to generate the pulse train.
//        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
//
//  value - The analong value, which translates to the duty cycle of 
//        the pulse train.  Range: 0-255
//        0 - 0% duty cycle (no pulses are generated, output is LOW)
//		  128 - 50% duty cycle (pulse train is HIGH 50% of the time)
//        255 - 100% duty cycle (pulse train is HIGH continuously)
//
// NOTES:
//  A pulse frequency of 100hz is requested, which will actually get 
//  us about 92 hz (with clock granularity), or 92 pulses per second.
//
inline void analogWrite(int pin, int value)
{
    _ValidatePwmPin(pin);
    _ValidatePinOkToChange(pin);
	_InitializePinIfNeeded(pin);

    HRESULT hr = ERROR_SUCCESS;
    ULONG dutyCycleIn = value & 255UL;	// Limit duty cycle to 100%

    // Scale the duty cycle to the range used by the driver.
    // From 0-255 to 0-PWM_MAX_DUTYCYCLE, rounding to nearest value.
    ULONG dutyCycle = ((dutyCycleIn * PWM_MAX_DUTYCYCLE) + 127UL) / 255UL;

    // If PWM operation is not currently enabled on this pin:
    if (!_pinData[pin].pwmIsEnabled)
    {
        // Prepare the pin for PWM use.
        _PinFunction(pin, _PwmMuxFunction[pin]);
        _SetImplicitPinMode(pin, OUTPUT);
        _pinData[pin].stateIsKnown = FALSE;

        // Start PWM on the pin.
        hr = PwmStart(_PwmPinMap[pin], PWM_HZ, dutyCycle);
        if (FAILED(hr))
        {
            ThrowError("PwmStart() failed. pin=%d, freq=100hz, dutyCycle=%d",
                pin, dutyCycleIn);
        }
        _pinData[pin].pwmIsEnabled = TRUE;
        _pinData[pin].pwmDutyCycle = dutyCycleIn;
    }

    // If PWM operation is enabled on this pin, and duty cycle is being changed:
    else if (_pinData[pin].pwmDutyCycle != dutyCycleIn)
    {
        hr = PwmSetDutyCycle(_PwmPinMap[pin], dutyCycle);
        if (FAILED(hr))
        {
            ThrowError("PwmSetDutyCycle() failed. pin=%d, dutyCycle=%d",
                pin, dutyCycleIn);
        }
        _pinData[pin].pwmDutyCycle = dutyCycleIn;
    }
}

class ArduinoStatic
{

public:
    ArduinoStatic() :
        adc(nullptr)
    { }

    ~ArduinoStatic()
    {
        if ( this->adc != nullptr )
        {
            AdcFree(this->adc);
            this->adc = nullptr;
        }
    }

    void begin()
    {
        if ( this->adc == nullptr )
        {
            HRESULT hr = AdcCreateInstance(ADC_CONTROLLER_INDEX, &this->adc);
            if ( FAILED(hr) )
            {
                ThrowError("Failed to initialized Analog");
            }
        }
    }

    //
    // Reads the value from the specified analog pin.
    // pin should be the analog pin number (0-5)
    //
    int analogRead(int pin)
    {
		LONG pinL = pin;

		// Allow for pins to be specified as A0-A5.
		if ((pinL >= A0) && (pinL <= A5))
		{
			pinL = pinL - A0;
		}

        // special value -1 means temp sense conversion
        if ( (pinL < -1) || (pinL >= NUM_ANALOG_PINS) )
        {
			ThrowError("Invalid pin number (%d). Pin must be in the range [-1, %d] or [A0, A5].\n",
                pinL, NUM_ANALOG_PINS);
        }

        if ( this->adc == nullptr )
        {
            ThrowError("Arduino not initialized");
        }

        LONG result = AdcSampleChannel(this->adc, (ULONG) pinL);
        if ( result < 0 )
        {
            ThrowError("AdcSampleChannel failed");
        }

        return result;
    }

private:
    ADC *adc;
};

__declspec (selectany) ArduinoStatic _ArduinoStatic;

inline void ArduinoInit()
{
    _ArduinoStatic.begin();

    // ensure level shifter enabled
    GpioSetDir(QRK_LEGACY_RESUME_SUS2, OUTPUT);
    GpioWrite(QRK_LEGACY_RESUME_SUS2, HIGH);
}

inline int analogRead(int channel)
{
    return _ArduinoStatic.analogRead(channel);
}

inline uint8_t shiftIn(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_)
{
    uint8_t buffer(0);

    for (uint8_t loop_count = 0, bit_index = 0 ; loop_count < 8 ; ++loop_count) {
        if (bit_order_ == LSBFIRST) {
            bit_index = loop_count;
        } else {
            bit_index = (7 - loop_count);
        }

        digitalWrite(clock_pin_, HIGH);
        buffer |= (digitalRead(data_pin_) << bit_index);
        digitalWrite(clock_pin_, LOW);
    }

    return buffer;
}

inline void shiftOut(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_, uint8_t byte_)
{
    for (uint8_t loop_count = 0, bit_mask = 0; loop_count < 8; ++loop_count) {
        if (bit_order_ == LSBFIRST) {
            bit_mask = (1 << loop_count);
        } else {
            bit_mask = (1 << (7 - loop_count));
        }

        digitalWrite(data_pin_, (byte_ & bit_mask));
        digitalWrite(clock_pin_, HIGH);
        digitalWrite(clock_pin_, LOW);
    }

    return;
}

//
// Arduino Sketch Plumbing
//

void setup();
void loop();

inline int RunArduinoSketch()
{
    int ret = 0;

    try
    {
        ArduinoInit();
        setup();
        while ( 1 )
        {
            loop();
        }
    }
    catch ( const _arduino_fatal_error &ex )
    {
        ret = 1;
        Log(L"\nSketch Aborted! A fatal error has occurred:\n");
        printf("%s\n", ex.what());
    }
    catch ( const _arduino_quit_exception & )
    {
        // exit cleanly
    }

    return ret;
}

//
// Initialize pseudo random number generator with seed
//
inline void randomSeed(unsigned int seed)
{
    if (seed != 0) {
        _WindowsRandom.Seed(seed);
    }
}

//
// Generate pseudo random number with upper bound max
//
inline long random(long max)
{
    if (max == 0) {
        return 0;
    }
    return _WindowsRandom.Next() % max;
}

//
// Generate pseudo random number in the range min - max
//
inline long random(long min, long max)
{
    if (min >= max) {
        return min;
    }
    long diff = max - min;
    return random(diff) + min;
}

inline uint16_t makeWord(unsigned char h, unsigned char l) { return (h << 8) | l; }
#define word(x, y) makeWord(x, y)

// Bits and Bytes
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define bit(b) (1UL << (b))
#define __attribute__(x)

#include "Stream.h"
#include "HardwareSerial.h"
#include "Wire.h"
#endif // _WINDOWS_ARDUINO_H_
