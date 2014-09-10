// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.


#pragma once

typedef void (*InterruptFunction)(void);

#ifndef INTERUPT_FREQUENCY
#define INTERUPT_FREQUENCY 100
#endif


//! noInterrupts
//! stubbed as this does nothing
inline void noInterrupts()
{

}

//! interrupts
//! stubbed as this does nothing
inline void interrupts()
{

}

//! Attach Fake Interrupt to a pin
//! Hardware interrupts are not currently implemented. This function will set up a fake interrupt
//! using a polling mechanism. 
//! \param pin - pin to attach an interrupt to
//! \param fxn - funcion to call
//! \param mode - one of 
//! * LOW	-	 triggers whenever the pin is low
//! * CHANGE	- triggers when the pin changes value
//! * RISING	- triggers when the pin changes from low to high
//! * Falling	- triggers when the pin changes from high to low
void attachInterrupt(uint8_t pin, InterruptFunction fxn, int mode);

//! Detatch Fake Interrupt
//! Disables the function for this interrupt
void detachInterrupt(uint8_t pin);

