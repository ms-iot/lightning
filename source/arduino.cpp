#include "arduino.h"

char* dtostrf(double value, char width, uint8_t precision, char* buffer)
{
	// dtostrf is defined that the buffer is width plus null
	sprintf_s(buffer, width + 1, "%*.*d", width, precision, value);

	return buffer;
}


void tone(int pin, unsigned int frequency)
{
    UNREFERENCED_PARAMETER(pin);
    UNREFERENCED_PARAMETER(frequency);

}

void tone(int pin, unsigned int frequency, unsigned long duration)
{
    UNREFERENCED_PARAMETER(pin);
    UNREFERENCED_PARAMETER(frequency);
    UNREFERENCED_PARAMETER(duration);
}

void noTone(int pin)
{
    UNREFERENCED_PARAMETER(pin);
}
