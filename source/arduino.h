// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _WINDOWS_ARDUINO_H_
#define _WINDOWS_ARDUINO_H_

// Arduino compatibility header for inclusion by user programs

#include <windows.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "ArduinoCommon.h"
#include "ArduinoError.h"
#include "WindowsRandom.h"
#include "WindowsTime.h"
#include "GalileoPins.h"
#include "binary.h"
#include "wire.h"
#include "Adc.h"

#define NUM_ARDUINO_PINS 20
#define NUM_ANALOG_PINS 6

#define GALILEO_A0      14

#define ARDUINO_CLOCK_SPEED 16000000UL    // 16 Mhz

//
// Definition of Constants as defined on http://wiring.org.co/reference/index.html and http://arduino.cc/en/Reference/Constants
//

#define PI              3.1415926535897932384626433832795
#define HALF_PI         1.5707963267948966192313216916398
#define TWO_PI          6.283185307179586476925286766559

#define boolean bool
typedef uint8_t byte;

typedef unsigned short word;

//
// Printf like function to log to stdout and if a debugger is attached to the debugger output.
//
inline int Log(const char *format, ...)
{
    va_list args;
    int len = 0;
    char *buffer = NULL;

    va_start(args, format);
    len = _vscprintf(format, args) + 1;
    buffer = new char[len];
    if (buffer != NULL)
    {
        len = vsprintf_s(buffer, len, format, args);
        printf(buffer);
        if (IsDebuggerPresent())
        {
            OutputDebugStringA(buffer);
        }
        delete[](buffer);
    }
    else
    {
        len = 0;
    }
    return len;
}

inline int Log(const wchar_t *format, ...)
{
    va_list args;
    int len = 0;
    wchar_t *buffer = NULL;

    va_start(args, format);
    len = _vscwprintf(format, args) + 1;
    buffer = new wchar_t[len];
    if (buffer != NULL)
    {
        len = vswprintf_s(buffer, len, format, args);
        wprintf(buffer);
        if (IsDebuggerPresent())
        {
            OutputDebugStringW(buffer);
        }
        delete[](buffer);
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

    if (!g_pins._verifyPinFunction(pin, FUNC_DIO, GalileoPinsClass::NO_LOCK_CHANGE))
    {
        ThrowError("Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, GetLastError());
    }

    if (state != LOW)
    {
        // Emulate Arduino behavior here. Code like firmata uses bitmasks to set
        // ports, and will pass something like value & 0x20 and expect that to be high.
        state = HIGH;
    }

    if (!g_pins._setPinState(pin, state))
    {
        ThrowError("Error occurred setting pin: %d to state: %d, Error: %08x", pin, state, GetLastError());
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
    ULONG readData = 0;

    _ValidateArduinoPinNumber(pin);

    if (!g_pins._verifyPinFunction(pin, FUNC_DIO, GalileoPinsClass::NO_LOCK_CHANGE))
    {
        ThrowError("Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, GetLastError());
    }

    if (!g_pins._getPinState(pin, readData))
    {
        ThrowError("Error occurred reading pin: %d Error: %08x", pin, GetLastError());
    }

    return readData;
}

/// The number of bits used to return digitized analog values.
__declspec (selectany) ULONG g_analogValueBits = 10;

/// Perform an analog to digital conversion on one of the analog inputs.
/**
\param[in] pin The analog pin to read (A0-A5).
\return Digitized analog value read from the pin.
\note The number of bits of the digitized analog value can be set by calling the 
analogReadResolution() API.  By default ten bits are returned (0-1023 for 0-5v pin voltage).
\sa analogReadResolution
*/
inline int analogRead(int pin)
{
    ULONG value;
    ULONG bits;
    ULONG ioPin;
    ULONG chan;

    // Translate the pin number passed in to an I/O Pin number and a channel number.
    if ((pin >= 0) && (pin < NUM_ANALOG_PINS))
    {
        ioPin = A0 + pin;
        chan = pin;
    }
    else if ((pin >= A0) && (pin <= A5))
    {
        ioPin = pin;
        chan = pin - A0;
    }
    else
    {
        ThrowError("Pin: %d is not an analog input pin.", pin);
    }

    if (!g_pins._verifyPinFunction(ioPin, FUNC_AIN, GalileoPinsClass::NO_LOCK_CHANGE))
    {
        ThrowError("Error occurred verifying pin: %d function: ANALOG_IN, Error: 0x%08x", ioPin, GetLastError());
    }

    if (!g_adc.readValue(chan, value, bits))
    {
        ThrowError("Error performing analogRead on pin: %d, Error: 0x%08x", pin, GetLastError());
    }

    // Scale the digitized analog value to the currently set analog read resolution.
    if (g_analogValueBits > bits)
    {
        value = value << (g_analogValueBits - bits);
    }
    else if (bits > g_analogValueBits)
    {
        value = value >> (bits - g_analogValueBits);
    }

    return value;
}

/// Analog reference value.
#define DEFAULT 0

/// Set the number of bits returned by an analogRead() call.
/**
\param[in] bits The number of bits returned from an analogRead() call.
\note If more bits are specified than are natively produced by the ADC on the board
the digitized analog values are padded with zeros.  If fewer bits are specified, analog
values truncated to the desired length.
*/
inline void analogReadResolution(int bits)
{
    if ((bits < 1) || (bits > 32))
    {
        ThrowError("Attempt to set analog read resolution to %d bits.  Supported range: 1-32.", bits);
    }
    g_analogValueBits = bits;
}

/// Set the reference voltage used for analog inputs.
/**
The Galileo only supports an internal 5v reference.  Attempting to select any other
reference than DEFAULT throws an error.
\param[in] type The type of analong reference desired.
\note DEFAULT - ok, INTERNAL, INTERNAL1V1, INTERNAL2V56 or EXTERNAL - error.
*/
inline void analogReference(int type)
{
    if (type != DEFAULT)
    {
        ThrowError("The only supported analog reference is DEFAULT.");
    }
}

/// Configure a pin for input or output duty.
/**
\param[in] pin The number of the pin (D0-D13, A0, A5)
\param[in] mode The desired pin mode (INPUT, OUTPUT, INPUT_PULLUP)
*/
inline void pinMode(unsigned int pin, unsigned int mode)
{
    switch (mode)
    {
    case INPUT:
        if (!g_pins._setPinMode(pin, DIRECTION_IN, false))
        {
            ThrowError("Error setting mode: INPUT for pin: %d, Error: 0x%08x", pin, GetLastError());
        }
        break;
    case OUTPUT:
        if (!g_pins._setPinMode(pin, DIRECTION_OUT, false))
        {
            ThrowError("Error setting mode: OUTPUT for pin: %d, Error: 0x%08x", pin, GetLastError());
        }
        break;
    case INPUT_PULLUP:
        if (!g_pins._setPinMode(pin, DIRECTION_IN, true))
        {
            ThrowError("Error setting mode: INPUT_PULLUP for pin: %d, Error: 0x%08x", pin, GetLastError());
        }
        break;
    default:
        ThrowError("Invalid mode: %d specified for pin: %d.", mode, pin);
    }
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

#include "HardwareSerial.h"

void setup();
void loop();

#ifdef SERIAL_EVENT
void serialEvent();
#endif
#ifdef SERIAL_EVENT1
void serialEvent1();
#endif

inline int RunArduinoSketch()
{
    int ret = 0;

    try
    {
        //ArduinoInit();
        setup();
        while ( 1 )
        {
            loop();
            #ifdef SERIAL_EVENT
            if (Serial && Serial.available() > 0)
            {
                serialEvent();
            }
            #endif
            #ifdef SERIAL_EVENT1
            if (Serial1 && Serial1.available() > 0)
            {
                serialEvent1();
            }
            #endif
        }
    }
    catch ( const _arduino_fatal_error &ex )
    {
        ret = 1;
        Log("\nSketch Aborted! A fatal error has occurred:\n");
        Log("%s\n", ex.what());
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

inline uint16_t makeWord(uint8_t h, uint8_t l) { return (h << 8) | l; }
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

#include "Wire.h"
#endif // _WINDOWS_ARDUINO_H_
