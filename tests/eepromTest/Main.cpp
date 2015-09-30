// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"
#include "eeprom.h"

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

void setup()
{
    EEPROM.write(1979, 'Z');
    EEPROM.write(1987, '@');
    EEPROM.write(1995, 'K');
    EEPROM.write(2003, '!');

    Log("Read \"%c\" from the EEPROM.", EEPROM.read(1979));
}

void loop()
{
}