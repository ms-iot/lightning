// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "arduino.h"
#include "Servo.h"


///
/// \brief Creates a servo object
///
Servo::Servo()
    : _servoIndex(0)
    , _min(MIN_PULSE_WIDTH)
    , _max(MAX_PULSE_WIDTH)
    , _attachedPin(-1)
{

}

///
/// \brief Attaches a servo instance to a pin
/// \details This will designate which pin the Servo instance will change.
/// \param [in] pin - The Arduino GPIO pin on which to generate the pulse.
///        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
///
uint8_t Servo::attach(int pin)
{
    _attachedPin = pin;
    _min = MIN_PULSE_WIDTH;
    _max = MAX_PULSE_WIDTH;

    return 0;
}

///
/// \brief Attaches a servo instance to a pin
/// \details This will designate which pin the Servo instance will change.
/// \param [in] pin - The Arduino GPIO pin on which to generate the pulse.
///        This can be pin 3, 5, 6, 7, 8, 9, 10, or 11.
/// \param [in] min - The minimum value for write calls to the Servo
/// \param [in] max - The maximum value for write calls to the Servo
///
uint8_t Servo::attach(int pin, int min, int max)
{
    _attachedPin = pin;
    _min = min;
    _max = max;

    return 0;
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
    if (!attached())
    {
        ThrowError("Error when calling write, servo is not attached.\n");
        return;
    }

    // value is in angles and needs to be converted to microSeconds
    if (value <= 0)
    {
        writeMicroseconds(_min);
    }
    else if (value >= 180)
    {
        writeMicroseconds(_max);
    }
    else
    {
        double alternateValue = (double) value / 180 * (_max - _min) + _min;
        writeMicroseconds((int) alternateValue);
    }
}

///
/// \brief Writes a value to a servo instance on it's attached pin
/// \details This will start a pulse on the attached pin that will cause a servo to
///        to go to the angle respective to the give value unless it is a continuous rotation servo.
/// \param [in] value - the microseconds for the pulse to be on in a 20 ms period
///
void Servo::writeMicroseconds(int value)
{
    if (!attached())
    {
        ThrowError("Error when calling writeMicroseconds, servo is not attached.\n");
        return;
    }

    // validating that the value inputted is within the bounds of the min and max
    int alternateValue = value;
    if (value < _min)
    {
        alternateValue = _min;
    }
    else if (value > _max)
    {
        alternateValue = _max;
    }

    // Making the frequency 50 Hz which is equal to a 20ms period
    int frequency = (int)((double) 1 / ((double) REFRESH_INTERVAL / 1000000));
    
    // Validation of the pin to make sure PWM functionality is allowed
    if (!g_pins.verifyPinFunction(_attachedPin, FUNC_PWM, GalileoPinsClass::NO_LOCK_CHANGE))
    {
        ThrowError("Error occurred verifying pin: %d function: PWM, Error: %08x", _attachedPin, GetLastError());
    }

    HRESULT hr = ERROR_SUCCESS;

    // Scale the duty cycle to the range used by the driver.
    // From 0-255 to 0-PWM_MAX_DUTYCYCLE, rounding to nearest value.
    ULONG dutyCycle = (ULONG) ((((double) alternateValue / REFRESH_INTERVAL * 255UL * PWM_MAX_DUTYCYCLE) + 127UL) / 255UL);

    // Prepare the pin for PWM use.
    if (!g_pins.setPwmDutyCycle(_attachedPin, (ULONG) dutyCycle))
    {
        ThrowError("Error occurred setting pin: %d PWM duty cycle to: %d, Error: %08x", _attachedPin, dutyCycle, GetLastError());
    }

    double servoIndexDouble = (double) (alternateValue - _min) / (_max - _min) * 180;
    _servoIndex = static_cast<uint8_t>(servoIndexDouble);
}

///
/// \brief Returns the last value written to the servo in angles
///
int Servo::read()
{
    return _servoIndex;
}

///
/// \brief Returns the last value written to the servo in microseconds
///
int Servo::readMicroseconds()
{
    return (int)(((double) _servoIndex / 180) * (_max - _min) + _min);
}

///
/// \brief Returns a boolean value describing whether or not the
///        Servo instance has been attached to a pin
///
bool Servo::attached()
{
    if (_attachedPin == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
