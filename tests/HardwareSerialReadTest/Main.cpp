// Main.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "arduino.h"

int _tmain(int argc, _TCHAR* argv [])
{
    return RunArduinoSketch();
}

void setup()
{
    Serial.begin(CBR_300, Serial.SERIAL_7O2);
    //Serial.end();
}

void loop()
{
    // Reads characters
    int c = Serial.read();
    if (c < 0)
    {
        Log(L"Serial.read had no data\n");
    }
    else{
        Log(L"Loop: %c\n", c);
    }

    // Reads Integer
    //Log(L"ParseInt: %d\n", Serial.parseInt());

    // Reads Float
    //Log(L"ParseFloat: %f\n", Serial.parseFloat());
}

void serialEvent()
{
    //int c = Serial.read();
    //if (c < 0)
    //{
    //    Log(L"Serial.read had no data\n");
    //}
    //else{
    //    Log(L"serialEvent: %d\n", c);
    //}
}