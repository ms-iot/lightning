// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "arduino.h"
#include "Servo.h"

Servo::Servo()
    : _servoIndex(0)
    , _min(MIN_PULSE_WIDTH)
    , _max(MAX_PULSE_WIDTH)
    , _attachedPin(-1)
{

}

uint8_t Servo::attach(int pin)
{
    _attachedPin = pin;
    _min = MIN_PULSE_WIDTH;
    _max = MAX_PULSE_WIDTH;

    return 0;
}

uint8_t Servo::attach(int pin, int min, int max)
{
    _attachedPin = pin;
    _min = min;
    _max = max;
    return 0;
}

void Servo::detach()
{
    _attachedPin = -1;
}

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

void Servo::writeMicroseconds(int value)
{
    if (!attached())
    {
        ThrowError("Error when calling writeMicroseconds, servo is not attached.\n");
        return;
    }
    // On standard servos a parameter value of 1000 is fully counter-clockwise, 2000 is fully clockwise, and 1500 is in the middle.
    int frequency = (double) 1 / ((double)REFRESH_INTERVAL / 1000000);

    // Generates and starts the square wave of designated frequency at 50% duty cycle
    // Cannot generate tones lower than 31Hz
    _ValidatePwmPin(_attachedPin);
    _ValidatePinOkToChange(_attachedPin);
    _InitializePinIfNeeded(_attachedPin);

    HRESULT hr = ERROR_SUCCESS;

    // Scale the duty cycle to the range used by the driver.
    // From 0-255 to 0-PWM_MAX_DUTYCYCLE, rounding to nearest value.
    ULONG dutyCycle = (((double)value / REFRESH_INTERVAL * 255UL * PWM_MAX_DUTYCYCLE) + 127UL) / 255UL;

    // If PWM operation is not currently enabled on this pin:
    if (!_pinData[_attachedPin].pwmIsEnabled)
    {
        // Prepare the pin for PWM use.
        _PinFunction(_attachedPin, _PwmMuxFunction[_attachedPin]);
        _SetImplicitPinMode(_attachedPin, OUTPUT);
        _pinData[_attachedPin].stateIsKnown = FALSE;

        // Start PWM on the pin.
        hr = PwmStart(_PwmPinMap[_attachedPin], frequency, dutyCycle);
        if (FAILED(hr))
        {
            ThrowError("PwmStart() failed. pin=%d, freq=%d", _attachedPin, frequency);
        }
        _pinData[_attachedPin].pwmIsEnabled = TRUE;
        _pinData[_attachedPin].pwmDutyCycle = dutyCycle;
    }
    // If PWM operation is enabled on this pin:
    else
    {
        // Since we have no driver function to change a PWM Frequency, we will have to stop it and restart it with the new frequency.
        PwmStop(_PwmPinMap[_attachedPin]);
        hr = PwmStart(_PwmPinMap[_attachedPin], frequency, dutyCycle);
        if (FAILED(hr))
        {
            ThrowError("PwmStart() failed. pin=%d, freq=%d", _attachedPin, frequency);
        }
        _pinData[_attachedPin].pwmDutyCycle = dutyCycle;
    }

    double servoIndexDouble = (double) (value - _min) / (_max - _min) * 180;
    _servoIndex = (int) servoIndexDouble;

    // TODO: get rid of this logging
    Log("value=%d\n", value);
    Log("pin=%d, freq=%d, dutyCycle=%d\n", _attachedPin, frequency, dutyCycle);
    Log("servoIndex=%lf\n", servoIndexDouble);
}

int Servo::read()
{
    // returns the current angle of the servo
    return _servoIndex;
}

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
