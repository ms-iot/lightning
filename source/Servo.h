// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#define MIN_PULSE_WIDTH 544
#define MAX_PULSE_WIDTH 2400
#define DEFAULT_PULSE_WIDTH 1500
#define REFRESH_INTERVAL 20000 // 20 ms

#define MAX_SERVOS 16

class Servo
{
    int _pin[16];    // Servo Library can support up to 12 motors

    int _attachedPin;

    uint8_t _servoIndex;
    int _min;
    int _max;

public:
    Servo();
    uint8_t attach(int pin);
    uint8_t attach(int pin, int min, int max);
    void detach();
    void write(int angle);
    void writeMicroseconds(int value);
    int read();
    bool attached();

};