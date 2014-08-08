// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once

#include "Arduino.h"

/* 
	Name: pulseIn()

	Reference:
		http://arduino.cc/en/Reference/PulseIn

	Description:

		Read a pulse on a pin(HIGH or LOW), and returns the duration of the pulse in microseconds.
		returns 0 if the timeout is exceeded while waiting for a pulse.

	Syntax:

		pulseIn(iPin, iValue)
		pulseIn(iPin, iValue, ulTimeout)

	Parameters:

		iPin: pin number to read the pulse (int)
		iValue: pulse to read: HIGH or LOW (int)
		ulTimeout: wait time, in microseconds, for pulse start; 1 second default (unsigned long)
*/

unsigned long pulseIn(int iPin, int iValue, unsigned long ulTimeout = 1000000UL);

/*
	Name: _Duration()

	Description:

		Helper function. calculates the duration between start time and end time, compensating for timer overflow

	Syntax:

		_Duration(startTime, endTime)

	Parameters:

		startTime: start time in microseconds (unsigned long)
		endTime: end time in microsseconds (unsigned long)
	
	Returns:

		the duration between start time and end time (unsigned long)
*/

unsigned long _Duration(unsigned long ulStartTime, unsigned long ulEndTime);