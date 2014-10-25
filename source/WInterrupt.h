/** \file winterrupt.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.  
 * See License.txt in the project root for license information.
 */

#pragma once

typedef void (*InterruptFunction)(void);

#ifndef INTERRUPT_FREQUENCY
/// \brief Default interrupt update frequency
/// \note We will tune this based on feedback. If you have a
/// specific scenario where you need finer grained interrupt
/// handling, you can change this in the project settings by
/// adding INTERRUPT_FREQUENCY=<em>number; to the preprocessor
/// definitions
#define INTERRUPT_FREQUENCY 100
#endif


/// \brief Turn off or disable interrupts
/// \warning not implemented
/// \see <a href="http://arduino.cc/en/Reference/NoInterrupts" target="_blank">origin: Arduino::noInterrupts</a>
inline void noInterrupts()
{

}

/// \brief Enable/resume interrupts
/// \warning not implemented
/// \see <a href="http://arduino.cc/en/Reference/Interrupts" target="_blank">origin: Arduino::interrupts</a>
inline void interrupts()
{

}

/// \brief Attach software interrupt to a pin
/// \details Hardware interrupts are not currently implemented.
/// This function will set up a thread to poll the pin and trigger
/// an interrupt handler when activated.
/// \param [in] pin Pin to attach an interrupt to
/// \param [in] fxn Funcion to call
/// \param [in] mode - trigger behavior
/// \n Valid values:
/// \arg LOW - triggers whenever the pin is low
/// \arg CHANGE - triggers when the pin changes value
/// \arg RISING - triggers when the pin changes from low to high
/// \arg FALLING - triggers when the pin changes from high to low
/// \see <a href="http://arduino.cc/en/Reference/AttachInterrupt" target="_blank">origin: Arduino::attachInterrupt</a>
void attachInterrupt(uint8_t pin, InterruptFunction fxn, int mode);

//! Detatch Fake Interrupt
//! Disables the function for this interrupt
/// \see <a href="http://arduino.cc/en/Reference/DetachInterrupt" target="_blank">origin: Arduino::detachInterrupt</a>
void detachInterrupt(uint8_t pin);
