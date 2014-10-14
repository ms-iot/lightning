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
    Serial1.begin(CBR_9600, Serial.SERIAL_8N1);
	Serial.begin(CBR_300, Serial.SERIAL_7O2);
	//Serial.print("Serial Beginning");
	char temp[] = "Serial Beginning";
	Log("character array written: %d\n", Serial1.write(temp));
    Log("const char *: %d\n", Serial1.write("Serial Beginning"));
}

int count = 0;
char c = 'a';
uint8_t dataArray[5] = { 0x03, 0x03, 0x00, 0x03, 0x03 };

void loop()
{
    for (count = 0; count < 5; count++)
    {
        if (Serial1.write(dataArray[count]) != 1)
        {
            Log("Error writing Serial1! %d\n", count);
        }

        if (Serial.write(dataArray[count]) != 1)
        {
            Log("Error writing Serial! %d\n", count);
        }
    }

    Sleep(500);
    int available = Serial.available();
    if (available)
    {
        Log("Received %d bytes\n", available);
        for (int i = 0; i < available; i++)
        {
            auto byte = (uint8_t) Serial.read();
            Log("%X,", byte);
        }
    }

    // Sending characters
    Serial1.write(c);
    if (Serial.write(c) != 1)
    {
        Log(L"Serial.write failed\n");
    }
    else
    {
        Log(L"%c being sent\n", c);
    }

    if (c == 'z')
    {
        c = 'a';
    }
    else
    {
        c++;
    }

    // Sending negative integer in string
    Serial.print("-100");

    // Sending integers in string
    Serial.write('0' + count);
    Log(L"%c being sent\n", '0' + count++);
    if (count == 10) { count = 0; }

    // Sending float in string
    Serial.print("3.14159");
    Log(L"Sending Pi\n");

    // Sending integer
    int a = -2147483647;
    unsigned int a2 = 4294967295;

    Log(L"Sending int\n");
    Serial.write(a);
    // spacer
    Log(L"Sending spacer\n");
    Serial.write(5);
    Log(L"Sending unsigned int\n");
    Serial.write(a2);

    // spacer
    Log(L"Sending spacer\n");
    Serial.write(10);

    // Sending long
    long n = -2147483647;
    unsigned long n2 = 4294967295;

    Log(L"Sending long\n");
    Serial.write(n);
    Log(L"Sending spacer\n");
    Serial.write(5);
    Log(L"Sending unsigned long\n");
    Serial.write(n2);

    Sleep(2000);
}