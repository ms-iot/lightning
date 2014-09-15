// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

int led = 10;  // This is the pin the LED is attached to.

void setup()
{
    // TODO: Add your code here
    
    pinMode(led, OUTPUT);       // Configure the pin for OUTPUT so you can turn on the LED.
}

// the loop routine runs over and over again forever:
void loop()
{
    // Default resolution is 8 bits - Max value 255
    analogWrite(led, 255);
    Log("%u/65535 s/b ~100%%\n", _pinData[led].pwmDutyCycle);
    analogWriteResolution(10);  // Max value 1023
    analogWrite(led, 255);
    Log("%u/65535 s/b ~25%%\n", _pinData[led].pwmDutyCycle);
    analogWriteResolution(12);  // Max value 4095
    analogWrite(led, 4095);
    Log("%u/65535 s/b ~100%%\n", _pinData[led].pwmDutyCycle);
    throw _arduino_quit_exception();
}