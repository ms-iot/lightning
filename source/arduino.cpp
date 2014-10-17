#include "arduino.h"

char* dtostrf(double value, char width, uint8_t precision, char* buffer)
{
	// dtostrf is defined that the buffer is width plus null
	sprintf_s(buffer, width + 1, "%*.*d", width, precision, value);

	return buffer;
}

