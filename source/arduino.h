// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _WINDOWS_ARDUINO_H_
#define _WINDOWS_ARDUINO_H_

// Arduino compatibility header for inclusion by user programs

#include <winsock2.h>
#include <WS2tcpip.h>
#include <strsafe.h>
#include <stdint.h>
#include <stdexcept>

#define _USE_MATH_DEFINES
#include <math.h>

#include "ArduinoError.h"
#include "WindowsRandom.h"
#include "WindowsTime.h"
#include "embprpusr.h"
#include "galileo.h"
#include "binary.h"

#define NUM_ARDUINO_PINS 20
#define NUM_ANALOG_PINS 6

#define GALILEO_A0      14

#define NOT_MUXED       0x0
#define DEFAULT_MUX     0x0
#define ALTERNATE_MUX   0x1

#define ARDUINO_CLOCK_SPEED 16000000UL    // 16 Mhz

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

void inline _RevertPinToDigital(int pin);

// Type of struct used to store state and configuration information
// for a GPIO pin.  Storing this information allows pins to be configured 
// with a minimum number of transfers to the I/O Expander and also allows 
// pin configurations to be restored when needed after changes have been 
// made for an alternate pin function (PWM vs GPIO use, for example).
typedef struct {
    UINT pwmDutyCycle : 8;      // Pulse width (0 = 0%, 255 = 100%)
    UINT currentMode : 1;       // INPUT or OUTPUT
    UINT modeSet : 1;           // INPUT or OUTPUT set explicitely
    UINT currentMux : 1;        // DEFAULT_MUX or ALTERNATE_MUX
    UINT muxSet : 1;            // DEFAULT_MUX or ALTERNATE_MUX set explicitely
    UINT state : 1;             // LOW or HIGH
    BOOL stateIsKnown : 1;      // TRUE or FALSE
    BOOL pwmIsEnabled : 1;      // TRUE or FALSE
    BOOL pinIsLocked : 1;       // TRUE or FALSE
    BOOL pinInUseSpi : 1;       // TRUE or FALSE
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
    GPORT0_BIT2_PWM3,			// Pin 12 - PWM 3
    0,							// Pin 13 - No PWM
    0,							// Pin 14 - No PWM
    0,							// Pin 15 - No PWM
    0,							// Pin 16 - No PWM
    0,							// Pin 17 - No PWM
    0,							// Pin 18 - No PWM
    0							// Pin 19 - No PWM
};

// This array stores state and configuration information for each GPIO pin.
__declspec (selectany) PIN_DATA _pinData[NUM_ARDUINO_PINS];

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
inline void digitalWrite(int pin, int state)
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
inline void pinMode(int pin, int mode)
{
    _ValidateArduinoPinNumber(pin);
    _ValidatePinOkToChange(pin);

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
inline void _SetImplicitPinMode(int pin, int mode)
{
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
inline void _RevertImplicitPinMode(int pin)
{
    pinMode(pin, _pinData[pin].modeSet);
}

// Select a particular function on a multiplexed IO pin in such a way that it 
// can be reverted later if needed.
// This function assumes the pin value has already been checked for range.
inline bool _SetImplicitPinFunction(ULONG pin, ULONG function)
{
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
inline bool pinFunction(ULONG pin, ULONG function)
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
    pinFunction(pin, _pinData[pin].muxSet);
}

// Revert the pin to digital I/O use if needed.
// This function assumes the caller has range-checked the pin number.
void inline _RevertPinToDigital(int pin)
{
    _ValidatePinOkToChange(pin);

    if (_pinData[pin].pwmIsEnabled)
    {
        PwmStop(pin);
        _pinData[pin].pwmIsEnabled = FALSE;
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

    HRESULT hr = ERROR_SUCCESS;
    ULONG dutyCycleIn = value & 255UL;	// Limit duty cycle to 100%

    // Scale the duty cycle to the range used by the driver.
    // From 0-255 to 0-PWM_MAX_DUTYCYCLE, rounding to nearest value.
    ULONG dutyCycle = ((dutyCycleIn * PWM_MAX_DUTYCYCLE) + 127UL) / 255UL;

    // If PWM operation is not currently enabled on this pin:
    if (!_pinData[pin].pwmIsEnabled)
    {
        // Prepare the pin for PWM use.
        pinFunction(pin, _PwmMuxFunction[pin]);
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
        // special value -1 means temp sense conversion
        if ( (pin < -1) || (pin >= NUM_ANALOG_PINS) )
        {
            ThrowError("Invalid pin number (%d). Pin must be in the range [-1, %d).\n",
                pin, NUM_ANALOG_PINS);
        }

        if ( this->adc == nullptr )
        {
            ThrowError("Arduino not initialized");
        }

        LONG result = AdcSampleChannel(this->adc, pin);
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

    // Initialize all pins to their default function and set all as inputs
    for ( int i = 0; i < NUM_ARDUINO_PINS; ++i )
    {
        pinMode(i, INPUT);
        pinFunction(i, DEFAULT_MUX);
        _pinData[i].currentMode = INPUT;
        _pinData[i].currentMux = DEFAULT_MUX;
        _pinData[i].modeSet = INPUT;
        _pinData[i].muxSet = DEFAULT_MUX;
        _pinData[i].pinInUseSpi = FALSE;
        _pinData[i].pinIsLocked = FALSE;
        _pinData[i].pwmDutyCycle = 0;
        _pinData[i].pwmIsEnabled = FALSE;
        _pinData[i].state = 0;
        _pinData[i].stateIsKnown = FALSE;
    }
}

inline int analogRead(int channel)
{
    return _ArduinoStatic.analogRead(channel);
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

inline uint16_t makeWord(unsigned int w) { return w; }
inline uint16_t makeWord(unsigned char h, unsigned char l) { return (h << 8) | l; }


// Bits and Bytes
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define bit(b) (1UL << (b))
#define __attribute__(x)

#ifdef USE_NETWORKSERIAL
#include "NetworkSerial.h"
#else
#include "HardwareSerial.h"
#endif

#include "Stream.h"
#include "Wire.h"
#endif // _WINDOWS_ARDUINO_H_
