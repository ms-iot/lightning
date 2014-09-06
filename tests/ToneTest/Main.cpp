// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"

int _tmain(int argc, _TCHAR* argv [])
{
    return RunArduinoSketch();
}

int tonePin = 3;
int frequency = 300;

void setup()
{
    Log(L"Tone\n");
    tone(tonePin, 100, 2000);
}

void loop()
{
    //Log(L"Tone On: %d\n", frequency);
    //tone(tonePin, frequency);
    //Sleep(500);
    //Log(L"Tone Off\n");
    //noTone(tonePin);
    //frequency = frequency + 50;
}
