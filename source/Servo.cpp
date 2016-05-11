// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "arduino.h"
#include "Servo.h"


///
/// \brief Creates a servo object
///
Servo::Servo() :
    _min(MIN_PULSE_WIDTH),
    _max(MAX_PULSE_WIDTH),
    _attachedPin(-1),
    _currentPulseMicroseconds(0),
    _actualPeriodMicroseconds(0)
{

}

///
/// \brief Attaches a servo instance to a pin
/// \details This will designate which pin the Servo instance will change.
/// \param [in] pin - The PWM pin on which to generate the pulse (PWM0 - PWM15)
///
void Servo::attach(int pin)
{
    attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

///
/// \brief Attaches a servo instance to a pin, specifying pulse width range.
/// \details This will designate which pin the Servo instance will change.
/// \param [in] pin - The PWM pin on which to generate the pulse (PWM0 - PWM15)
/// \param [in] min - The minimum microseconds for servo pulses, range: 0 - (max-1) )
/// \param [in] max - The maximum microseconds for servo pulses, range: (min+1) - 10000
///
void Servo::attach(int pin, int min, int max)
{
    HRESULT hr;
    ULONG ioPin;
    BoardPinsClass::BOARD_TYPE board;
    ULONG actualPwmFrequency = 0;

    if ((min < 0) || (max < (min + 1)) || (max > 10000))
    {
        ThrowError(E_INVALIDARG, "Servo pulse microsecond range specified is invalid");
    }
    _min = min;
    _max = max;

    hr = g_pins.getBoardType(board);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error getting board type.  Error: 0x%08x", hr);
    }

    switch (board)
    {
    case BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE:
        // The pin number passed in is a GPIO Pin number, use it as is.
        ioPin = pin;

        // Verify the pin is in PWM mode, and configure it for PWM use if not.
        hr = g_pins.verifyPinFunction(ioPin, FUNC_PWM, BoardPinsClass::LOCK_FUNCTION);
        if (FAILED(hr))
        {
            ThrowError(hr, "Error occurred verifying pin: %d function: PWM, Error: %08x", ioPin, hr);
        }
        break;

    case BoardPinsClass::BOARD_TYPE::MBM_BARE:
    case BoardPinsClass::BOARD_TYPE::PI2_BARE:
        // Translate the PWM channel numbers to fake pin numbers.
        if (pin < PWM0)
        {
            ioPin = PWM0 + pin;
        }
        else
        {
            ioPin = pin;
        }
        break;

    default:
        ThrowError(E_INVALIDARG, "Unrecognized board type: 0x%08x", board);
    }

    // Set the frequency on the PWM channel.
    hr = g_pins.setPwmFrequency(ioPin, SERVO_FREQUENCY_HZ);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred setting PWM frequency for servo use.");
    }

    // Record the information we need to drive a servo signal on the PWM pin.
    actualPwmFrequency = g_pins.getActualPwmFrequency(ioPin);
    _actualPeriodMicroseconds = (1000000 + (actualPwmFrequency / 2)) / actualPwmFrequency;
    _attachedPin = ioPin;

}

///
/// \brief Detaches a servo instance from a pin
///
void Servo::detach()
{
    _attachedPin = -1;
}

///
/// \brief Writes a value to a servo instance on it's attached pin
/// \details This will start a pulse on the attached pin that will cause a servo to
///        to go to the given angle unless it is a continuous rotation servo.
/// \param [in] value - the angle for the servo to turn to
///
void Servo::write(int value)
{
    ULONG pulseMicroseconds;

    if (!attached())
    {
        ThrowError(HRESULT_FROM_WIN32(ERROR_INVALID_STATE), "Error when calling write, servo is not attached.");
        return;
    }

    // Value is in degrees from 0-180, convert it to microSeconds
    if (value <= 0)
    {
        pulseMicroseconds = _min;
    }
    else if (value >= 180)
    {
        pulseMicroseconds = _max;
    }
    else
    {
        pulseMicroseconds = ( (((_max - _min) * value) + 90) / 180 ) + _min;
    }

    writeMicroseconds(pulseMicroseconds);
}

///
/// \brief Writes a value to a servo instance on it's attached pin
/// \details This will start a pulse on the attached pin that will cause a servo to
///        to go to the angle respective to the give value unless it is a continuous rotation servo.
/// \param [in] value - the microseconds for the pulse to be on in a 20 ms period
///
void Servo::writeMicroseconds(int value)
{
    ULONGLONG pulseMicroseconds;
    ULONG dutyCycle;
    HRESULT hr;

    if (!attached())
    {
        ThrowError(HRESULT_FROM_WIN32(ERROR_INVALID_STATE), "Error when calling writeMicroseconds, servo is not attached.");
        return;
    }

    // Limit the pulse microseconds to the range previously specifed as the min and max.
    if (value < _min)
    {
        pulseMicroseconds = _min;
    }
    else if (value > _max)
    {
        pulseMicroseconds = _max;
    }
    else
    {
        pulseMicroseconds = value;
    }

    // Scale the duty cycle to the range used by the driver (0-0xFFFFFFFF)
    dutyCycle = (ULONG)(((pulseMicroseconds * 0xFFFFFFFFLL) + (ULONGLONG)(_actualPeriodMicroseconds / 2)) / ((ULONGLONG)_actualPeriodMicroseconds));

    // Prepare the pin for PWM use.
    hr = g_pins.setPwmDutyCycle(_attachedPin, (ULONG)dutyCycle);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred setting pin: %d PWM duty cycle to: %d, Error: %08x", _attachedPin, dutyCycle, hr);
    }

    // Record the currently set pulse time in microseconds.
    _currentPulseMicroseconds = (ULONG)pulseMicroseconds;
}

///
/// \brief Returns the last value written to the servo in angles
///
int Servo::read()
{
    int servoDegrees;
    servoDegrees = (((_currentPulseMicroseconds - _min) * 180) + ((_max - _min) / 2)) / (_max - _min);
    return servoDegrees;
}

