// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "Arduino.h"
#include "PulseIn.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long pulseIn(int iPin, int iValue, unsigned long ulTimeout)
{
	//
	// start and end times
	//

	unsigned long ulStartTime = 0;
	unsigned long ulEndTime = 0;

	//
	// start the timer
	//

	ulStartTime = micros();

	//
	// clear out the non desired reads to find the start of the pulese (LOW if iValue is HIGH, HIGH if iValue is LOW)
	//

	while (LOW == iValue ? HIGH == digitalRead(iPin) : LOW == digitalRead(iPin))
	{
		//
		// with each cycle, see if we have waited longer then the designated timeout
		//

		ulEndTime = micros();

		if (_Duration(ulStartTime, ulEndTime) > ulTimeout)
		{
			//
			// in the case of a timeout, the duration needs to be 0
			//

			ulStartTime = 0;
			ulEndTime = 0;

			//
			// this was a timeout expected error, so end the function
			//

			goto CleanUp;
		}
	}

	//
	// start the timer
	//

	ulStartTime = micros();

	//
	// keep looping until the recieve pin is not high anymore
	//

	while (iValue == digitalRead(iPin))
	{
		//
		// no code here
		//
	}

	//
	// get the amount of time the recieve pin was the expected value
	//

	ulEndTime = micros();

CleanUp:

	//
	// return the pulse length duration, compensating for timer overflow
	//

	return _Duration(ulStartTime, ulEndTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long _Duration(unsigned long ulStartTime, unsigned long ulEndTime)
{
	unsigned long ulCurrentTime = 0;
	unsigned long ulDuration = 0;

	//
	// see if the there was a timer overflow
	//

	if (ulEndTime < ulStartTime)
	{
		//
		// there was a timer overflow, so compensate
		//

		ulCurrentTime = ULONG_MAX - ulStartTime;
		ulDuration = ulEndTime + ulCurrentTime;
	}
	else
	{
		//
		// calculate the final duration 
		//

		ulDuration = ulEndTime - ulStartTime;
	}

	//
	// return the calculated duration
	//

	return ulDuration;
}
