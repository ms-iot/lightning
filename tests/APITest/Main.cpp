// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"

#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    return RunArduinoSketch();
}

// Check build output (/W4) as well as log ("Output" tab below).
void setup()
{
    // Call/test degree() and radians() global functions
    float deg(85.0f);
    deg = ::degrees(::radians(deg));
    Log("CHECK: %s | ::degrees() and ::radians()\n", (deg > 84.9 && deg < 85.1) ? "Pass" : "FAILED");

    // Test binary.h inclusion from arduino-sdk
    uint8_t byte = B10101010;

    // Call/test ...
    // next test here
}

void loop()
{
    _exit_arduino_loop();
}
