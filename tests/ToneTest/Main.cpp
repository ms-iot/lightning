// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"

int _tmain(int argc, _TCHAR* argv [])
{
    return RunArduinoSketch();
}

int tonePin = 3;
int tonePinTwo = 5;
int frequency = 300;

void setup()
{
    Log(L"Tone\n");
    tone(tonePin, 100, 2000);
}

void loop()
{
    // Calling tone twice in a row on different pins
    Log(L"Calling tone twice in a row on different pins\n");
    Log(L"Tone On: %d\n", frequency);
    tone(tonePin, frequency);
    Sleep(500);
    Log(L"Tone2 On: %d\n", frequency);
    tone(tonePinTwo, frequency);
    Sleep(500);

    // Calling tone twice in a row on the same pin
    Log(L"Calling tone twice in a row on the same pin\n");
    Log(L"Tone On: %d\n", frequency);
    tone(tonePin, frequency);
    Sleep(500);
    frequency = frequency + 50;
    Log(L"Tone On: %d\n", frequency);
    tone(tonePin, frequency);
    Sleep(500);
    frequency = frequency + 50;

    // Calling tone and no tone one after another
    Log(L"Calling tone and no tone one after another\n");
    Log(L"Tone On: %d\n", frequency);
    tone(tonePin, frequency);
    Sleep(500);
    Log(L"Tone Off\n");
    noTone(tonePin);
    frequency = frequency + 50;

    // Calling noTone twice in a row
    Log(L"noTone twice in a row\n");
    Sleep(500);
    Log(L"Tone Off\n");
    noTone(tonePin);
    frequency = frequency + 50;

    // Calling tone with a duration and noTone before it's timer
    Log(L"Calling tone with a duration and noTone before it's timer\n");
    tone(tonePin, 100, 2000);
    noTone(tonePin);

    // Calling tone with a duration and letting the callback hit
    Log(L"Calling tone with a duration and letting the callback hit\n");
    tone(tonePin, 100, 2000);
    Sleep(3000);
}
