// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "arduino.h"

Servo::Servo()
: _servoIndex(0)
, _min(MIN_PULSE_WIDTH)
, _max(MAX_PULSE_WIDTH)
{

}

uint8_t Servo::attach(int pin)
{
    pin;
}

uint8_t Servo::attach(int pin, int min, int max)
{
    pin; min; max;
}

void Servo::detach()
{

}

void Servo::write(int value)
{
    value;
}

void Servo::writeMicroseconds(int value)
{
    value;
}

int Servo::read()
{
    return 0;
}

int Servo::readMicroseconds()
{
    return 0;
}

bool Servo::attached()
{
    return false;
}
