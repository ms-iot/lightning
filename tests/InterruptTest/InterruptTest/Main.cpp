// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

int led = 13;  // This is the pin the LED is attached to.
int ledLow = 9;
int ledChange = 10;
bool on = false;
bool onLow = false;

void lightUp()
{
    digitalWrite(led, HIGH);
}

void lightOff()
{
    digitalWrite(led, LOW);
    detachInterrupt(3);
}

void lightLOW()
{
    digitalWrite(ledLow, onLow?HIGH:LOW);
    onLow = !onLow;
}

void lightChange()
{
    digitalWrite(ledChange, on ? HIGH : LOW);
    on = !on;
}

void setup()
{
    // TODO: Add your code here
    
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(led, OUTPUT);
    pinMode(ledLow, OUTPUT);
    pinMode(ledChange, OUTPUT);

    digitalWrite(led, LOW);
    digitalWrite(ledLow, LOW);
    digitalWrite(ledChange, LOW);

    attachInterrupt(3, lightUp, RISING);
    attachInterrupt(4, lightOff, FALLING);
    attachInterrupt(5, lightLOW, LOW);
    attachInterrupt(6, lightChange, CHANGE);
}


// the loop routine runs over and over again forever:
void loop()
{
}