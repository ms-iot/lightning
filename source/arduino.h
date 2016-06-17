// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _WINDOWS_ARDUINO_H_
#define _WINDOWS_ARDUINO_H_

// Arduino compatibility header for inclusion by user programs
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Lightning.h>
#include <windows.h>
#include <devioctl.h>

#ifdef USE_NETWORKSERIAL
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

#include <algorithm>
#include <cstdint>
#include <memory>
#include <map>
#include <vector>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "ArduinoCommon.h"
#include "ArduinoError.h"
#include "WindowsRandom.h"
#include "WindowsTime.h"
#include "BoardPins.h"
#include "binary.h"
#include "wire.h"
#include "Adc.h"
#include "pins_arduino.h"
#include "PulseIn.h"
#include "WString.h"

#include "avr/macros.h"

#define NUM_ARDUINO_PINS 20
#define NUM_ANALOG_PINS 6

#define ARDUINO_CLOCK_SPEED 16000000UL    // 16 Mhz

//
// Definition of Constants as defined on http://wiring.org.co/reference/index.html and http://arduino.cc/en/Reference/Constants
//

#define PI              M_PI
#define HALF_PI         M_PI_2
#define TAU             (M_PI * 2.0f)
#define TWO_PI          TAU

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
#define constrain(amt,low,high) do {amt=((amt)<(low)?(low):((amt)>(high)?(high):(amt)));} while (0)
#define sq(x) ((x)*(x))
inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//
// Pauses the program for the amount of time (in microseconds) 
// specified as parameter.
//
LIGHTNING_DLL_API void delayMicroseconds(unsigned int us);

//
// Pauses the program for the amount of time (in miliseconds) 
// specified as parameter.
//
LIGHTNING_DLL_API void delay(unsigned long ms);

// Returns the number of milliseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 50 days.
LIGHTNING_DLL_API unsigned long millis(void);

// Returns the number of microseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 70 minutes.
LIGHTNING_DLL_API unsigned long micros(void);

//
// Returns true if an Arduino pin number is also an analog input
//
inline bool _IsAnalogPin(int num)
{
    return num >= A0;
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
LIGHTNING_DLL_API void digitalWrite(unsigned int pin, unsigned int state);

//
// Reads the value from the digital pin (IO0 - IO13).
// A0-A5 are mapped to 14-19
//
// Return Value:
//
// 1 for HIGH, 0 for LOW or error
// 
// Example:
//
//  // read IO4.
//  int val = digitalRead(4);
//
LIGHTNING_DLL_API int digitalRead(int pin);

/// The number of bits used to return digitized analog values.
LIGHTNING_DLL_API extern ULONG g_analogValueBits;

/// Perform an analog to digital conversion on one of the analog inputs.
/**
\param[in] pin The analog pin to read (A0-A5, or 0-5).
\return Digitized analog value read from the pin.
\note The number of bits of the digitized analog value can be set by calling the 
analogReadResolution() API.  By default ten bits are returned (0-1023 for 0-5v pin voltage).
\sa analogReadResolution
*/
LIGHTNING_DLL_API int analogRead(int pin);

/// Analog reference value.
#define DEFAULT 0

/// Set the number of bits returned by an analogRead() call.
/**
\param[in] bits The number of bits returned from an analogRead() call.
\note If more bits are specified than are natively produced by the ADC on the board
the digitized analog values are padded with zeros.  If fewer bits are specified, analog
values truncated to the desired length.
*/
LIGHTNING_DLL_API void analogReadResolution(int bits);

/// Set the reference voltage used for analog inputs.
/**
The Arduino only supports an internal 5v reference.  Attempting to select any other
reference than DEFAULT throws an error.
\param[in] type The type of analong reference desired.
\note DEFAULT - ok, INTERNAL, INTERNAL1V1, INTERNAL2V56 or EXTERNAL - error.
*/
LIGHTNING_DLL_API void analogReference(int type);

/// The number of bits used to specify PWM duty cycles.
LIGHTNING_DLL_API extern ULONG g_pwmResolutionBits;

/// Set the PWM duty cycle for a pin.
/**
\param[in] pin The number of the pin for the PWM output.  On boards with built-in PWM support
this is a GPIO pin, on boards that use an external PWM chip, this is a pseudo pin number named 
PWM0-PWMn, where "n" is one less than the number of PWM pins.
\param[in] dutyCycle The high pulse time, range 0 to pwm_resolution_count - 1, (defaults 
to a count of 255, for 8-bit PWM resolution.)
\Note: This call throws an error if the pin number is outside the range supported
on the board, or if a pin that does not support PWM is specified.
*/
LIGHTNING_DLL_API void analogWrite(unsigned int pin, unsigned int dutyCycle);

/// Set the number of bits used to specify PWM duty cycles to analogWrite().
/**
\param[in] bits The number of bits to use for analogWrite() duty cycle values.
*/
LIGHTNING_DLL_API void analogWriteResolution(int bits);

/// Configure a pin for input or output duty.
/**
\param[in] pin The number of the pin (D0-D13, A0, A5)
\param[in] mode The desired pin mode (INPUT, OUTPUT, INPUT_PULLUP)
*/
LIGHTNING_DLL_API void pinMode(unsigned int pin, unsigned int mode);

LIGHTNING_DLL_API uint8_t shiftIn(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_);

LIGHTNING_DLL_API void shiftOut(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_, uint8_t byte_);

///
/// \brief Performs a tone operation.
/// \details This will start a PWM wave on the designated pin of the
/// inputted frequency with 50% duty cycle
/// \param [in] pin - The Arduino GPIO pin on which to generate the pulse train.
///        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
/// \param [in] frequency - in Hertz
///
LIGHTNING_DLL_API void tone(int pin, unsigned int frequency);

///
/// \brief Performs a tone operation.
/// \details This will start a PWM wave on the designated pin of the
/// inputted frequency with 50% duty cycle and set up a timer to trigger
/// a callback after the inputted duration
/// \param [in] pin - The Arduino GPIO pin on which to generate the pulse train.
///        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
/// \param [in] frequency - in Hertz
/// \param [in] duration - in milliseconds
///
LIGHTNING_DLL_API void tone(int pin, unsigned int frequency, unsigned long duration);

///
/// \brief Performs a noTone operation.
/// \details This will stop a PWM wave on the designated pin if there is
/// a tone running on it
/// \param [in] pin - The Arduino GPIO pin on which to generate the pulse train.
///        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
///
LIGHTNING_DLL_API void noTone(int pin);

//
// Interrupt functions.
//

/// Attach a callback routine to a GPIO interrupt.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
*/
LIGHTNING_DLL_API void attachInterrupt(uint8_t pin, std::function<void(void)> func, int mode);

/// Attach a callback routine to a GPIO interrupt, with return of interrupt information.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
*/
LIGHTNING_DLL_API void attachInterruptEx(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, int mode);

/// Attach a callback routine to a GPIO interrupt, with return of interrupt information.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
\param[in] context An optional parameter to pass to the callback function.
*/
LIGHTNING_DLL_API void attachInterruptContext(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, void* context, int mode);

/// Indicate GPIO interrupt callbacks are no longer wanted for a pin.
/**
\param[in] pin The number of the board pin for which interrupts are to be detached.
*/
LIGHTNING_DLL_API void detachInterrupt(uint8_t pin);

/// Turn back on interrupt callbacks that have previously been disabled.
LIGHTNING_DLL_API void interrupts();

/// Temporarily disable delivery of all interrupt callbacks.
LIGHTNING_DLL_API void noInterrupts();

// Translate a board pin number to an interrupt number.  We use the same value
// for both (lower software layers translate the pin number to a SOC GPIO number).
inline uint8_t digitalPinToInterrupt(uint8_t pin)
{
    return pin;
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

// Arduino Sketch Plumbing
//
inline int RunArduinoSketch()
{
    int ret = 0;

    try
    {
        setup();
        while (1)
        {
            // This call is used to handle async procedure calls (APCs); usually by timers
            // This call will relinquish the remainder of its time slice to another 
            // ready to run thread of equal priority. However, in practice it is 
            // a no-op unless there's a pending APC. 
            SleepEx(0, TRUE);

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

    catch (const _arduino_fatal_error &ex)
    {
        ret = 1;
        Log("\nSketch Aborted! A fatal error has occurred:\n");
        Log("%s\n", ex.what());
    }
    catch (const _arduino_quit_exception &)
    {
        // exit cleanly
    }

    return ret;
}


//
// Initialize pseudo random number generator with seed
//
LIGHTNING_DLL_API void randomSeed(unsigned int seed);

//
// Generate pseudo random number with upper bound max
//
LIGHTNING_DLL_API long random(long max);

//
// Generate pseudo random number in the range min - max
//
LIGHTNING_DLL_API long random(long min, long max);

inline uint16_t makeWord(uint8_t h, uint8_t l) { return (h << 8) | l; }
#define word(x, y) makeWord(x, y)

// Bits and Bytes
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// Interrupt enable/disable stubs
#define cli()
#define sei()

#define bit(b) (1UL << (b))
#define __attribute__(x)


// Other utility Macros
// Turn passed in value into a string
#define STRINGIFY(x) #x
// Turn passed in macro into a string
#define STRINGIFY_MACRO(x) STRINGIFY(x)

inline float radians(float deg) { return deg * 180.0f / static_cast<float>(PI); }
inline float degrees(float rad) { return rad * static_cast<float>(PI) / 180.0f; }

#include "Wire.h"
#endif // _WINDOWS_ARDUINO_H_
