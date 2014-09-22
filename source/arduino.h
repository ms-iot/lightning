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
#include "WString.h"
#include "GalileoPins.h"
#include "binary.h"
#include "wire.h"

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
    if (!g_pins._setPinMode(pin, mode, false))
    {
        ThrowError("Error setting mode: %d for pin: %d, Error: %08x", mode, pin, GetLastError());
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

#include "Stream.h"
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
