// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _ARDUINO_COMMON_H_
#define _ARDUINO_COMMON_H_

#include <cstdint>

const UCHAR LOW = 0x00;
const UCHAR HIGH = 0x01;

const UCHAR DIRECTION_IN = 0x00;
const UCHAR DIRECTION_OUT = 0x01;
const UCHAR INPUT_PULLUP = 0x02;

// INPUT pin mode
#define INPUT DIRECTION_IN

// OUTPUT pin mode
#define OUTPUT DIRECTION_OUT

const UCHAR LSBFIRST = 0x00;
const UCHAR MSBFIRST = 0x01;

const UCHAR NUM_ARDUINO_PINS = 20;

const UCHAR CHANGE = 0x01;
const UCHAR FALLING = 0x02;
const UCHAR RISING = 0x03;

const uint16_t PWM_MAX_DUTYCYCLE = 0xffff;

// Pin name to number mapping.
const UCHAR D0 = 0;
const UCHAR D1 = 1;
const UCHAR D2 = 2;
const UCHAR D3 = 3;
const UCHAR D4 = 4;
const UCHAR D5 = 5;
const UCHAR D6 = 6;
const UCHAR D7 = 7;
const UCHAR D8 = 8;
const UCHAR D9 = 9;
const UCHAR D10 = 10;
const UCHAR D11 = 11;
const UCHAR D12 = 12;
const UCHAR D13 = 13;
const UCHAR A0 = 14;
const UCHAR A1 = 15;
const UCHAR A2 = 16;
const UCHAR A3 = 17;
const UCHAR A4 = 18;
const UCHAR A5 = 19;

// Pin function type values.
const UCHAR FUNC_NUL = 0x00;    ///< No function has been set
const UCHAR FUNC_DIO = 0x01;    ///< Digital I/O function
const UCHAR FUNC_PWM = 0x02;    ///< Pulse Width Modulation (PWM) function
const UCHAR FUNC_AIN = 0x04;    ///< Analog In function
const UCHAR FUNC_I2C = 0x08;    ///< I2C Bus function
const UCHAR FUNC_SPI = 0x10;    ///< SPI Bus function
const UCHAR FUNC_SER = 0x20;    ///< Hardware Serial function

// SPI signal to pin mapping.
const UCHAR PIN_MOSI = D11;     ///< Pin used for SPI MOSI signal
const UCHAR PIN_MISO = D12;     ///< Pin used for SPI MISO signal
const UCHAR PIN_SCK = D13;      ///< Pin used for SPI Clock signal

// I2C signal to pin mapping.
const UCHAR PIN_I2C_DAT = A4;   ///< Pin used for I2C Data signal
const UCHAR PIN_I2C_CLK = A5;   ///< Pin used for I2C Clock signal

#endif  // _ARDUINO_COMMON_H_