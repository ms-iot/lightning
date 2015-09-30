// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "stdafx.h"
#include "arduino.h"
#include "pulseIn.h"

//
// this test for pulseIn assumes a configuration such that an input that goes HIGH then LOW
// periodically is supplied to pin 3
//

static const int INPUT_PIN = 3;

//
// the amount of time to wait for the start of the pulse
//

static const unsigned long WAIT_TIMEOUT_MICROS = 5000000UL;

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

void setup()
{
	//
	// set the input pin for digital input
	//

	pinMode(INPUT_PIN, INPUT);

	//
	// perform the test: you have 5 seconds to do the transitions
	// 

	Log(L"Set input on pin %i LOW, then HIGH, then LOW\n", INPUT_PIN);
	Log(L"Duration: %u (microseconds)\n", pulseIn(INPUT_PIN, HIGH, WAIT_TIMEOUT_MICROS));

}

void loop()
{
	//
	// no code here
	//
}