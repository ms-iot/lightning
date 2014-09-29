// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"
#include "spi.h"

SPIClass spi = SPIClass();
/*
Uses pins:
MOSI: 11
MISO: 12
SCK: 13

*/

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

void setup()
{
    spi.begin();
    spi.setBitOrder(LSBFIRST);
    spi.transfer(10);
    spi.end();
}

// the loop routine runs over and over again forever:
void loop()
{
}