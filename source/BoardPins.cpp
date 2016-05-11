// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "ErrorCodes.h"
#include "BoardPins.h"
#include "I2c.h"

// The default PWM chip I2C address on the Ika Lure is 0x40.  To use the Ika Lure with a 
// Weather Shield which has a humidity sensor at addresss 0x40 the address of the PWM chip
// on the Ika Lure must be changed (by lifting the PWM chip pin #1 and tying it high,
// setting the PWM chip address to 0x41, for example).  If you change the PWM chip address
// by modifying the hardware, change the following #define to match.
#define IKA_LURE_PWM_I2C_ADR 0x41

// 
// Global extern exports
//
BoardPinsClass g_pins;

// GPIO type values.
const UCHAR GPIO_S0 = 1;        ///< GPIO is from the MBM SOC S0 sub-system
const UCHAR GPIO_S5 = 2;        ///< GPIO is from the MBM SOC S5 sub-system
const UCHAR GPIO_BCM = 3;       ///< GPIO is from the BCM2836 SOC GPIO sub-system
const UCHAR GPIO_NONE = 255;    ///< Specifies there is no GPIO pin with this number

// GPIO pin driver selection values.
const UCHAR GPIO_INPUT_DRIVER_SELECT = 1;   ///< Specify input circuitry should be enabled
const UCHAR GPIO_OUTPUT_DRIVER_SELECT = 0;  ///< Specify output driver should be enabled

// I/O Expander name values.
const UCHAR SOCBAYT    =  0;    ///< Value specifies BayTrail SOC is the "I/O Expander"
const UCHAR PWMI       =  1;    ///< Value specifies Ika Lure PWM used as I/O Expander
const UCHAR SOCBCM     =  2;    ///< Value specifies BCM2836 SOC is the "I/O Expander"
const UCHAR NO_X       = 15;    ///< Value specifies that no I/O Expander is used

// I/O Expander types.
const UCHAR PCA9685 = 0;        ///< PWM chip used on Gen2 and Ika Lure
const UCHAR BAYTRAIL = 1;       ///< Muxing is done within the MBM SOC
const UCHAR BCM2836 = 2;        ///< Muxing is done within the PI2 SOC

// PWM chip bit values.
const UCHAR LED0  =  0;         ///< PWM chip LED0 output
const UCHAR LED1  =  1;         ///< PWM chip LED1 output
const UCHAR LED2  =  2;         ///< PWM chip LED2 output
const UCHAR LED3  =  3;         ///< PWM chip LED3 output
const UCHAR LED4  =  4;         ///< PWM chip LED4 output
const UCHAR LED5  =  5;         ///< PWM chip LED5 output
const UCHAR LED6  =  6;         ///< PWM chip LED6 output
const UCHAR LED7  =  7;         ///< PWM chip LED7 output
const UCHAR LED8  =  8;         ///< PWM chip LED8 output
const UCHAR LED9  =  9;         ///< PWM chip LED9 output
const UCHAR LED10 = 10;         ///< PWM chip LED10 output
const UCHAR LED11 = 11;         ///< PWM chip LED11 output
const UCHAR LED12 = 12;         ///< PWM chip LED12 output
const UCHAR LED13 = 13;         ///< PWM chip LED13 output
const UCHAR LED14 = 14;         ///< PWM chip LED14 output
const UCHAR LED15 = 15;         ///< PWM chip LED15 output

//// MUX name values.
const UCHAR MUX0      =  0;     ///< Mux number 0
const UCHAR MUX1      =  1;     ///< Mux number 1
const UCHAR MUX2      =  2;     ///< Mux number 2
const UCHAR MUX3      =  3;     ///< Mux number 3
const UCHAR MUX4      =  4;     ///< Mux number 4
const UCHAR MUX5      =  5;     ///< Mux number 5
const UCHAR MUX6      =  6;     ///< Mux number 6
const UCHAR NO_MUX    = 15;     ///< Value indicates no mux is present

// Maximum number of MUXes supported in the tables.
const UCHAR MAX_MUXES = 15;     ///< Maximum number of MUXes on a Gen1 or Gen2 board.

// The number of GPIO pins on an MBM plus one (to allow for 0 not being used).
const ULONG NUM_MBM_PINS = 27;  ///< Number of entries in a zero based array indexed by MBM pin number.

// The I2C Address of the MBM Ika Lure ADC.
const ULONG MBM_IKA_LURE_ADC_ADR = 0x48;    ///< I2C address of ADC on MBM Ika Lure
                                            
// The number of connector pins on an PI2 plus one for zero, plus one for onboard led.
const ULONG NUM_PI2_PINS = 42;  ///< Number of entries in a zero based array indexed by PI2 pin number.

// The expected I2C address of an external PCA9685 PWM chip.
const UCHAR EXT_PCA9685_I2C_ADR = 0x40;


#if defined(_M_IX86) || defined(_M_X64)
/// The global table of pin attributes for the MBM board.
/**
This table contains all the pin-specific attributes needed to configure and use an I/O pin.
It is indexed by pin number (0 to NUM_MBM_PINS-1).
*/
const BoardPinsClass::PORT_ATTRIBUTES g_MbmPinAttributes[] =
{
    //gpioType           pullupExp   triStExp    muxA               Muxes (A,B) by function:    I2S  triStIn   Function_mask
    //             portBit     pullupBit   triStBit      muxB     Dio  Pwm  AnIn I2C  Spi  Ser     Spk   _pad
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  0
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  1
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  2
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  3
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  4
    { GPIO_S0,    17,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  //  5
    { GPIO_S0,     1,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  //  6
    { GPIO_S0,    18,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  //  7
    { GPIO_S0,     2,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  //  8
    { GPIO_S0,    19,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  //  9
    { GPIO_S0,     4,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  // 10
    { GPIO_S0,    16,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 11
    { GPIO_S0,     0,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  // 12
    { GPIO_S0,    20,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_I2C },  // 13
    { GPIO_S0,    13,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, 0, 0, FUNC_DIO | FUNC_I2S },  // 14
    { GPIO_S0,    21,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_I2C },  // 15
    { GPIO_S0,    12,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, 0, 0, FUNC_DIO | FUNC_I2S },  // 16
    { GPIO_S0,     7,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  // 17
    { GPIO_S0,    14,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, 0, 0, FUNC_DIO | FUNC_I2S },  // 18
    { GPIO_S0,     6,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  // 19
    { GPIO_S0,    15,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, 0, 0, FUNC_DIO | FUNC_I2S },  // 20
    { GPIO_S5,    29,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 21
    { GPIO_S0,    10,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },  // 22
    { GPIO_S5,    33,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 23
    { GPIO_S0,    11,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },  // 24
    { GPIO_S5,    30,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 25
    { GPIO_S0,   103,    NO_X, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 1, 0, 0, FUNC_DIO | FUNC_SPK }   // 26
};

/// The global table of mux attributes for the MBM board.
/**
This table contains the information needed to set each mux to a desired state.  It is indexed by
mux number and specifies which port bit of which I/O Expander drives the mux selection signal.
*/
const BoardPinsClass::MUX_ATTRIBUTES g_MbmMuxAttributes[MAX_MUXES] =
{
    { SOCBAYT,  0 },    ///< MUX0
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 },    // Not used on bare MBM
    { NO_X,     0 }     // Not used on bare MBM
};

/// The global table of PWM information for the MBM board.
/**
This table contains the information needed to drive the PWM channels.  It is indexed by the
connector pin number, and specifies the chip and port-bit that implements PWM for that pin.
*/
const BoardPinsClass::PWM_CHANNEL g_MbmPwmChannels[] =
{
    { NO_X,     0, 0, 0 },          ///<  0
    { NO_X,     0, 0, 0 },          ///<  1
    { NO_X,     0, 0, 0 },          ///<  2
    { NO_X,     0, 0, 0 },          ///<  3
    { NO_X,     0, 0, 0 },          ///<  4
    { NO_X,     0, 0, 0 },          ///<  5
    { NO_X,     0, 0, 0 },          ///<  6
    { NO_X,     0, 0, 0 },          ///<  7
    { NO_X,     0, 0, 0 },          ///<  8
    { NO_X,     0, 0, 0 },          ///<  9
    { NO_X,     0, 0, 0 },          ///< 10
    { NO_X,     0, 0, 0 },          ///< 11
    { NO_X,     0, 0, 0 },          ///< 12
    { NO_X,     0, 0, 0 },          ///< 13
    { NO_X,     0, 0, 0 },          ///< 14
    { NO_X,     0, 0, 0 },          ///< 15
    { NO_X,     0, 0, 0 },          ///< 16
    { NO_X,     0, 0, 0 },          ///< 17
    { NO_X,     0, 0, 0 },          ///< 18
    { NO_X,     0, 0, 0 },          ///< 19
    { NO_X,     0, 0, 0 },          ///< 20
    { NO_X,     0, 0, 0 },          ///< 21
    { SOCBAYT,  0, 0, 0 },          ///< 22
    { NO_X,     0, 0, 0 },          ///< 23
    { SOCBAYT,  1, 0, 0 },          ///< 24
    { NO_X,     0, 0, 0 },          ///< 25
    { NO_X,     0, 0, 0 }           ///< 26
};

/// The global table of pin attributes for the MBM board with an Ika Lure attached.
/**
This table contains all the pin-specific attributes needed to configure and use an I/O pin.
It is indexed by pin number (0 to NUM_ARDUINO_PINS-1).
*/
const BoardPinsClass::PORT_ATTRIBUTES g_MbmIkaPinAttributes[] =
{
    //gpioType           pullupExp   triStExp    muxA               Muxes (A,B) by function:    I2S  triStIn   Function_mask
    //             portBit     pullupBit   triStBit      muxB     Dio  Pwm  AnIn I2C  Spi  Ser     Spk   _pad
    { GPIO_S0,     6,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },            // D0
    { GPIO_S0,     7,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },            // D1
    { GPIO_S5,    33,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },                       // D2
    { GPIO_S0,    10,    NO_X, 0,    NO_X, 0,    MUX6,   MUX0,    0,0, 0,1, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },            // D3
    { GPIO_S5,    29,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },                       // D4
    { GPIO_S0,    11,    NO_X, 0,    NO_X, 0,    MUX6,   MUX1,    0,0, 0,1, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },            // D5
    { GPIO_S0,   103,    NO_X, 0,    NO_X, 0,    MUX6,   MUX2,    0,0, 0,1, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },            // D6
    { GPIO_S0,    15,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },                       // D7
    { GPIO_S0,    14,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },                       // D8
    { GPIO_S0,    13,    NO_X, 0,    NO_X, 0,    MUX6,   MUX3,    0,0, 0,1, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },            // D9
    { GPIO_S0,    17,    NO_X, 0,    NO_X, 0,    MUX6,   MUX4,    0,0, 0,1, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM },            // D10
    { GPIO_S0,    19,    NO_X, 0,    NO_X, 0,    MUX6,   MUX5,    0,0, 0,1, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_PWM | FUNC_SPI }, // D11
    { GPIO_S0,    18,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },            // D12
    { GPIO_S0,    16,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },            // D13
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_AIN },                       // A0
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_AIN },                       // A1
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_AIN },                       // A2
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_AIN },                       // A3
    { GPIO_S0,    21,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_I2C },                       // A4
    { GPIO_S0,    20,    NO_X, 0,    NO_X, 0,    MUX6,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_I2C }                        // A5
};

/// The global table of mux attributes for the MBM board with an Ika Lure attached.
/**
This table contains the information needed to set each mux to a desired state.  It is indexed by
mux number and specifies which port bit of which I/O Expander drives the mux selection signal.
*/
const BoardPinsClass::MUX_ATTRIBUTES g_MbmIkaMuxAttributes[MAX_MUXES] =
{
    { PWMI,     LED6 },     ///< MUX0
    { PWMI,     LED7 },     ///< MUX1
    { PWMI,     LED8 },     ///< MUX2
    { PWMI,     LED9 },     ///< MUX3
    { PWMI,     LED10 },    ///< MUX4
    { PWMI,     LED11 },    ///< MUX5
    { SOCBAYT,  0 },        ///< MUX6
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 },        // Not used on Ika Lure
    { NO_X,     0 }         // Not used on Ika Lure
};

/// The global table of PWM information for the MBM board with an Ika Lure attached.
/**
This table contains the information needed to drive the PWM channels.  It is indexed by the
connector pin number, and specifies the chip and port-bit that implements PWM for that pin.
*/
const BoardPinsClass::PWM_CHANNEL g_MbmIkaPwmChannels[] =
{
    { NO_X, 0, 0, 0 },          ///< D0
    { NO_X, 0, 0, 0 },          ///< D1
    { NO_X, 0, 0, 0 },          ///< D2
    { PWMI, LED0, LED0 , 0 },   ///< D3
    { NO_X, 0, 0, 0 },          ///< D4
    { PWMI, LED1, LED1, 0 },    ///< D5
    { PWMI, LED2, LED2, 0 },    ///< D6
    { NO_X, 0, 0, 0 },          ///< D7
    { NO_X, 0, 0, 0 },          ///< D8
    { PWMI, LED3, LED3, 0 },    ///< D9
    { PWMI, LED4, LED4, 0 },    ///< D10
    { PWMI, LED5, LED5, 0 },    ///< D11
    { NO_X, 0, 0, 0 },          ///< D12
    { NO_X, 0, 0, 0 },          ///< D13
    { NO_X, 0, 0, 0 },          ///< A0
    { NO_X, 0, 0, 0 },          ///< A1
    { NO_X, 0, 0, 0 },          ///< A2
    { NO_X, 0, 0, 0 },          ///< A3
    { NO_X, 0, 0, 0 },          ///< A4
    { NO_X, 0, 0, 0 }           ///< A5
};
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_ARM)
/// The global table of pin attributes for the PI2 board.
/**
This table contains all the pin-specific attributes needed to configure and use an I/O pin.
It is indexed by pin number (0 to NUM_PI2_PINS-1).
*/
const BoardPinsClass::PORT_ATTRIBUTES g_Pi2PinAttributes[] =
{
    //gpioType           pullupExp   triStExp    muxA               Muxes (A,B) by function:    I2S  triStIn   Function_mask
    //             portBit     pullupBit   triStBit      muxB     Dio  Pwm  AnIn I2C  Spi  Ser     Spk   _pad
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  0
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  1
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  2
    { GPIO_BCM,    2,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_I2C },  //  3
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  4
    { GPIO_BCM,    3,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 1,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_I2C },  //  5
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  6
    { GPIO_BCM,    4,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             //  7
    { GPIO_BCM,   14,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  //  8
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             //  9
    { GPIO_BCM,   15,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SER },  // 10
    { GPIO_BCM,   17,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 11
    { GPIO_BCM,   18,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 12
    { GPIO_BCM,   27,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 13
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 14
    { GPIO_BCM,   22,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 15
    { GPIO_BCM,   23,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 16
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 17
    { GPIO_BCM,   24,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 18
    { GPIO_BCM,   10,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 19
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 20
    { GPIO_BCM,    9,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 21
    { GPIO_BCM,   25,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 22
    { GPIO_BCM,   11,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 23
    { GPIO_BCM,    8,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 24
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 25
    { GPIO_BCM,    7,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 0, 0, 0, 0, FUNC_DIO | FUNC_SPI },  // 26
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 27
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 28
    { GPIO_BCM,    5,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 29
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 30
    { GPIO_BCM,    6,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 31
    { GPIO_BCM,   12,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 32
    { GPIO_BCM,   13,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 33
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 34
    { GPIO_BCM,   19,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 35
    { GPIO_BCM,   16,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 36
    { GPIO_BCM,   26,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 37
    { GPIO_BCM,   20,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 38
    { GPIO_NONE,   0,    NO_X, 0,    NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_NUL },             // 39
    { GPIO_BCM,   21,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO },             // 40
    { GPIO_BCM,   47,    MUX0, 0,    NO_X, 0,    MUX0,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, 0, 0, FUNC_DIO }              // 41 - LED
};

/// The global table of mux attributes for the PI2 board.
/**
This table contains the information needed to set each mux to a desired state.  It is indexed by
mux number and specifies which port bit of which I/O Expander drives the mux selection signal.
*/
const BoardPinsClass::MUX_ATTRIBUTES g_Pi2MuxAttributes[MAX_MUXES] =
{
    { SOCBCM,  0 },    ///< MUX0
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 },    // Not used on bare PI2
    { NO_X,    0 }     // Not used on bare PI2
};

/// The global table of PWM information for the PI2 board.
/**
This table contains the information needed to drive the PWM channels.  It is indexed by the
PI2 connector pin number, and specifies the chip and port-bit that implements PWM for that pin.
This is currently not used, but is included here for compatibility with the other boards.
*/
const BoardPinsClass::PWM_CHANNEL g_Pi2PwmChannels[] =
{
    { NO_X, 0, 0, 0 },          ///<  0
    { NO_X, 0, 0, 0 },          ///<  1
    { NO_X, 0, 0, 0 },          ///<  2
    { NO_X, 0, 0, 0 },          ///<  3
    { NO_X, 0, 0, 0 },          ///<  4
    { NO_X, 0, 0, 0 },          ///<  5
    { NO_X, 0, 0, 0 },          ///<  6
    { NO_X, 0, 0, 0 },          ///<  7
    { NO_X, 0, 0, 0 },          ///<  8
    { NO_X, 0, 0, 0 },          ///<  9
    { NO_X, 0, 0, 0 },          ///< 10
    { NO_X, 0, 0, 0 },          ///< 11
    { NO_X, 0, 0, 0 },          ///< 12
    { NO_X, 0, 0, 0 },          ///< 13
    { NO_X, 0, 0, 0 },          ///< 14
    { NO_X, 0, 0, 0 },          ///< 15
    { NO_X, 0, 0, 0 },          ///< 16
    { NO_X, 0, 0, 0 },          ///< 17
    { NO_X, 0, 0, 0 },          ///< 18
    { NO_X, 0, 0, 0 },          ///< 19
    { NO_X, 0, 0, 0 },          ///< 20
    { NO_X, 0, 0, 0 },          ///< 21
    { NO_X, 0, 0, 0 },          ///< 22
    { NO_X, 0, 0, 0 },          ///< 23
    { NO_X, 0, 0, 0 },          ///< 24
    { NO_X, 0, 0, 0 },          ///< 25
    { NO_X, 0, 0, 0 },          ///< 26
    { NO_X, 0, 0, 0 },          ///< 27
    { NO_X, 0, 0, 0 },          ///< 28
    { NO_X, 0, 0, 0 },          ///< 29
    { NO_X, 0, 0, 0 },          ///< 30
    { NO_X, 0, 0, 0 },          ///< 31
    { NO_X, 0, 0, 0 },          ///< 32
    { NO_X, 0, 0, 0 },          ///< 33
    { NO_X, 0, 0, 0 },          ///< 34
    { NO_X, 0, 0, 0 },          ///< 35
    { NO_X, 0, 0, 0 },          ///< 36
    { NO_X, 0, 0, 0 },          ///< 37
    { NO_X, 0, 0, 0 },          ///< 38
    { NO_X, 0, 0, 0 },          ///< 39
    { NO_X, 0, 0, 0 },          ///< 40
    { NO_X, 0, 0, 0 }           ///< 41 - PI2 Onboard LED
};
#endif // defined(_M_ARM)

/// The global table of I/O Expander attributes for the boards.
/**
This table contains the information needed to communicate with each I/O Expander chip.
It is indexed by Expander number and contains the type of chip used for that I/O Expander
and the address on the I2C bus the chip responds to.
*/
const BoardPinsClass::EXP_ATTRIBUTES g_GenxExpAttributes[] =
{
    { BAYTRAIL,  0x00 },                    ///< SOCBAYT - MBM
    { PCA9685,   IKA_LURE_PWM_I2C_ADR },    ///< PWMI - MBM with Ika Lure
    { BCM2836,   0x00 }                     ///< SOCBCM - PI2
};


/// The global table of Pin Function tracking structures.
/**
This table tracks the currently configured function for each pin of the board.  It must
contain at least the number of entries for the number of pins on the "largest" board.
*/
BoardPinsClass::PIN_FUNCTION g_GenxPinFunctions[] =
{
    { FUNC_NUL, false },    ///<  0
    { FUNC_NUL, false },    ///<  1
    { FUNC_NUL, false },    ///<  2
    { FUNC_NUL, false },    ///<  3
    { FUNC_NUL, false },    ///<  4
    { FUNC_NUL, false },    ///<  5
    { FUNC_NUL, false },    ///<  6
    { FUNC_NUL, false },    ///<  7
    { FUNC_NUL, false },    ///<  8
    { FUNC_NUL, false },    ///<  9
    { FUNC_NUL, false },    ///< 10
    { FUNC_NUL, false },    ///< 11
    { FUNC_NUL, false },    ///< 12
    { FUNC_NUL, false },    ///< 13
    { FUNC_NUL, false },    ///< 14
    { FUNC_NUL, false },    ///< 15
    { FUNC_NUL, false },    ///< 16
    { FUNC_NUL, false },    ///< 17
    { FUNC_NUL, false },    ///< 18
    { FUNC_NUL, false },    ///< 19
    { FUNC_NUL, false },    ///< 20
    { FUNC_NUL, false },    ///< 21
    { FUNC_NUL, false },    ///< 22
    { FUNC_NUL, false },    ///< 23
    { FUNC_NUL, false },    ///< 24
    { FUNC_NUL, false },    ///< 25
    { FUNC_NUL, false },    ///< 26
    { FUNC_NUL, false },    ///< 27
    { FUNC_NUL, false },    ///< 28
    { FUNC_NUL, false },    ///< 29
    { FUNC_NUL, false },    ///< 30
    { FUNC_NUL, false },    ///< 31
    { FUNC_NUL, false },    ///< 32
    { FUNC_NUL, false },    ///< 33
    { FUNC_NUL, false },    ///< 34
    { FUNC_NUL, false },    ///< 35
    { FUNC_NUL, false },    ///< 36
    { FUNC_NUL, false },    ///< 37
    { FUNC_NUL, false },    ///< 38
    { FUNC_NUL, false },    ///< 39
    { FUNC_NUL, false },    ///< 40
    { FUNC_NUL, false }     ///< 41 - PI2 Onboard LED
};


/// Constructor.
/**
Initialize the data members that point to the global attributes tables.
*/
BoardPinsClass::BoardPinsClass()
    :
    m_boardType(NOT_SET),
    m_PinAttributes(NULL),
    m_MuxAttributes(NULL),
    m_ExpAttributes(g_GenxExpAttributes),
    m_PinFunctions(g_GenxPinFunctions),
    m_PwmChannels(NULL),
    m_GpioPinCount(0)
{
}

/**
Method to determine whether a pin is set to a desired function or not, 
and to set it to that function if possible.
\param[in] pin The number of the pin in question
\param[in] function The desired function. See functions below.
\param[in] lockAction Desired lock action.  See lock actions below.
\return HRESULT success or error code.
\sa FUNC_DIO \sa FUNC_PWM \sa FUNC_AIN \sa FUNC_I2C \sa FUNC_SPI \sa FUNC_SER
\sa NO_LOCK_CHANGE \sa LOCK_FUNCTION \sa UNLOCK_FUNCTION
*/
HRESULT BoardPinsClass::verifyPinFunction(ULONG pin, ULONG function, FUNC_LOCK_ACTION lockAction)
{
    HRESULT hr = S_OK;
    BoardPinsClass::BOARD_TYPE board;

    // Make sure we know what type of board we are running on, needed by pinNumberIsSafe().
    hr = g_pins.getBoardType(board);

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = E_BOUNDS;
    }

    if (SUCCEEDED(hr) && (lockAction == UNLOCK_FUNCTION))
    {
        m_PinFunctions[pin].locked = false;
    }

    if (SUCCEEDED(hr) && (m_PinFunctions[pin].currentFunction != function))
    {
        if (m_PinFunctions[pin].locked)
        {
            hr = DMAP_E_PIN_FUNCTION_LOCKED;
        }
        else
        {
            hr = _setPinFunction(pin, function);

            if (SUCCEEDED(hr))
            {
                m_PinFunctions[pin].currentFunction = (UCHAR) function;
                if ((function == FUNC_SPI) || (function == FUNC_I2C))
                {
                    m_PinFunctions[pin].locked = true;
                }
            }
        }
    }

    if (SUCCEEDED(hr) && (lockAction == LOCK_FUNCTION))
    {
        m_PinFunctions[pin].locked = true;
    }

    return hr;
}

/**
This method prepares an external pin to be used for a specific function.
Functions are Digital I/O, Analog In, PWM, etc.
\param[in] pin the number of the pin in question.
\param[in] function the function to be used on the pin.
\return HRESULT success or error code.
\sa FUNC_DIO \sa FUNC_PWM \sa FUNC_AIN \sa FUNC_I2C \sa FUNC_SPI \sa FUNC_SER
*/
HRESULT BoardPinsClass::_setPinFunction(ULONG pin, ULONG function)
{
    HRESULT hr = S_OK;


    // Make sure the pin attributes table is set up for the board generation.
    hr = _verifyBoardType();

    if (SUCCEEDED(hr))
    {
        // Verify the pin number is in range.
        if (!pinNumberIsSafe(pin))
        {
            hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
        }
    }
    
    // Verify the requsted function is supported on this pin.
    if (SUCCEEDED(hr) && ((m_PinAttributes[pin].funcMask & function) == 0))
    {
        hr = DMAP_E_FUNCTION_NOT_SUPPORTED_ON_PIN;
    }

    if (SUCCEEDED(hr))
    {
        if (function == FUNC_DIO)
        {
            hr = _setPinDigitalIo(pin);
        }
        else if (function == FUNC_PWM)
        {
            hr = _setPinPwm(pin);
        }
        else if (function == FUNC_AIN)
        {
            hr = _setPinAnalogInput(pin);
        }
        else if (function == FUNC_I2C)
        {
            hr = _setPinI2c(pin);
        }
        else if (function == FUNC_SPI)
        {
            hr = _setPinSpi(pin);
        }
        else if (function == FUNC_SER)
        {
            hr = _setPinHwSerial(pin);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
    }

    return hr;
}

/**
This method sets the current function of a pin to Digial I/O.  This code
assumes the caller has verified the pin number is in range and that Digital
I/O is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinDigitalIo(ULONG pin)
{
    HRESULT hr = S_OK;
    
    // If the pin is tied to at least one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Digital I/O.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].digIoMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Digital I/O.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].digIoMuxB);
    }

    return hr;
}

/**
This method sets the current function of a pin to PWM.  This code
assumes the caller has verified the pin number is in range and that PWM
is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinPwm(ULONG pin)
{
    HRESULT hr = S_OK;

    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for PWM.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].pwmMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for PWM.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].pwmMuxB);
    }

    // Configure the pin for driving a PWM signal.
    if (SUCCEEDED(hr))
    {
        // Set the pin to be an output.
        hr = setPinMode(pin, DIRECTION_OUT, FALSE);
    }

    return hr;
}

/**
This method sets the current function of a pin to Analog Input.  This code
assumes the caller has verified the pin number is in range and that Analog
Input is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinAnalogInput(ULONG pin)
{
    HRESULT hr = S_OK;
 
    // If the pin is tied to at least one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Analog Input.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].anInMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Analog Input.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].anInMuxB);
    }

    // Make sure the digital I/O functions on this pin are set to INPUT without pull-up.
    if (SUCCEEDED(hr))
    {
        hr = setPinMode(pin, DIRECTION_IN, FALSE);
    }

    return hr;
}

/**
This method sets the current function of a pin to I2C Bus.  This code
assumes the caller has verified the pin number is in range and that I2C
bus use is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinI2c(ULONG pin)
{
    HRESULT hr = S_OK;

    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for I2C bus use.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].i2cMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for I2C bus use.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].i2cMuxB);
    }

    return hr;
}

/**
This method sets the current function of a pin to SPI Bus.  This code
assumes the caller has verified the pin number is in range and that SPI
bus use is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinSpi(ULONG pin)
{
    HRESULT hr = S_OK;

    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for SPI bus use.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].spiMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for SPI bus use.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].spiMuxB);
    }

    return hr;
}

/**
This method sets the current function of a pin to Hardware Serial.  This code
assumes the caller has verified the pin number is in range and that Hardware 
Serial is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setPinHwSerial(ULONG pin)
{
    HRESULT hr = S_OK;
    
    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Hardware Serial use.
        hr = _setMux(pin, m_PinAttributes[pin].muxA, m_PinAttributes[pin].serMuxA);
    }

    // If the pin is tied to a second MUX:
    if (SUCCEEDED(hr) && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Hardware Serial use.
        hr = _setMux(pin, m_PinAttributes[pin].muxB, m_PinAttributes[pin].serMuxB);
    }

    return hr;
}

/**
This method sets the mode and drive type of a pin (Input, Output, etc.)
\param[in] pin The number of the pin in question.
\param[in] mode The desired mode: DIRECTION_IN or DIRECTION_OUT.
\param[in] pullup True to enable pin pullup resistor, false to disable pullup
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::setPinMode(ULONG pin, ULONG mode, BOOL pullup)
{
    HRESULT hr = S_OK;

    if ((mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        hr = DMAP_E_INVALID_PIN_DIRECTION;
    }

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Set the pin direction on the device that supports this pin.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            hr = g_bcmGpio.setPinDirection(m_PinAttributes[pin].portBit, mode);
            break;
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            hr = g_btFabricGpio.setS0PinDirection(m_PinAttributes[pin].portBit, mode);
            break;
        case GPIO_S5:
            hr = g_btFabricGpio.setS5PinDirection(m_PinAttributes[pin].portBit, mode);
            break;
#endif // defined(_M_IX86) || defined(_M_X64)
        case GPIO_NONE:
            break;             // No actual GPIO pin here, nothing to do.
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Configure the pin drivers as needed.
        hr = _configurePinDrivers(pin, mode);
    }

    if (SUCCEEDED(hr))
    {
        // Configure the pin pullup as requested.
        hr = _configurePinPullup(pin, pullup);
    }

    return hr;
}

/**
This method sets the state of an I/O Expander port pin.
\param[in] pin The number of the GPIO pin being configured.
\param[in] expNo The number of the I/O Expander in question.
\param[in] bitNo Specifies the port and bit number to set (such as: P1_4 for port 1, bit 4)
\param[in] state The desired state: HIGH or LOW.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setExpBitToState(ULONG pin, ULONG expNo, ULONG bitNo, ULONG state)
{
    HRESULT hr = S_OK;
    UCHAR i2cAdr = 0;               // I2C address of I/O Expander

    // Get the I2C Address of the I/O Expander in question.
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    //
    // Determine what type of I/O Expander we are dealing with.
    //

    switch (m_ExpAttributes[expNo].Exp_Type)
    {
    case PCA9685:
        // Set the bit of the PWM chip to the desired state.
        hr = PCA9685Device::SetBitState(i2cAdr, bitNo, state);
        break;
#if defined(_M_ARM)
    case BCM2836:
        hr = g_bcmGpio.setPinFunction(m_PinAttributes[pin].portBit, state);
        break;
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
    case BAYTRAIL:
        if (m_PinAttributes[pin].gpioType == GPIO_S0)
        {
            hr = g_btFabricGpio.setS0PinFunction(m_PinAttributes[pin].portBit, state);
        }
        else
        {
            hr = g_btFabricGpio.setS5PinFunction(m_PinAttributes[pin].portBit, state);
        }
        break;
#endif // defined(_M_IX86) || defined(_M_X64)
    default:
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    return hr;
}

/**
Method to configure the pin input and output drivers to reflect the desired direction of a pin.
This code assumes the caller has verified the pin number and mode to be in range.
\param[in] pin The number of the pin in question.
\param[in] mode The desired direction for the pin.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_configurePinDrivers(ULONG pin, ULONG mode)
{
    HRESULT hr = S_OK;
    UCHAR expNo = 0;                // I/O Expander number
    UCHAR bitNo = 0;                // Bit number on I/O Expander
    UCHAR state = 0;                // Desired pin state

    if ((mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        hr = DMAP_E_INVALID_PIN_DIRECTION;
    }

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr))
    {
        // Determine which I/O Expander controls the GPIO-pin drivers.
        expNo = m_PinAttributes[pin].triStExp;
        // If there is an I/O Expander that controls the this pin drivers.
        if (expNo != NO_X)
        {
            // Determine which bit on the expander controls the drivers.
            bitNo = m_PinAttributes[pin].triStBit;

            // Determine the Exp pin state to select the needed driver.
            state = m_PinAttributes[pin].triStIn;
            if (mode == DIRECTION_OUT)
            {
                state = state ^ 0x01;
            }

            // Set the I/O Expander bit to the correct state.
            hr = _setExpBitToState(pin, expNo, bitNo, state);
        }
    }

    return hr;
}

/**
Method to configure the pin pullup as specified.  This code assumes the caller has
verified the pin number to be in the valid range.
\param[in] pin The number of the pin in question.
\param[in] pullup True to turn pullup on, false to turn pullup off.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_configurePinPullup(ULONG pin, BOOL pullup)
{
    HRESULT hr = S_OK;

    hr = _verifyBoardType();

    if (SUCCEEDED(hr))
    {
#if defined (_M_ARM)
        hr = g_bcmGpio.setPinPullup(m_PinAttributes[pin].portBit, pullup);
#endif // defined (_M_ARM)

        // Nothing is needed here for MBM pins.  MBM GPIO pins have pullups
        // from the level converters, whether they are wanted or not.
    }

    return hr;
}

/**
Method to set a MUX to select the desired signal.
\parma[in] pin The number of the pin being configured.
\param[in] mux The number of the MUX in question.
\param[in] selection The desired state of the MUX "select" input (LOW or HIGH).
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_setMux(ULONG pin, ULONG mux, ULONG selection)
{
    HRESULT hr = S_OK;
    ULONG expNo = 0;                // I/O Expander number
    ULONG bitNo = 0;                // Bit number on I/O Expander

    // If the MUX number is outside the valid range for this board, fail.
    if ((mux >= MAX_MUXES) || (m_MuxAttributes[mux].selectExp == NO_X))
    {
        hr = DMAP_E_DMAP_INTERNAL_ERROR;
    }

    if (SUCCEEDED(hr))
    {
        // Determine which I/O Expander and pin drive the MUX select input.
        expNo = m_MuxAttributes[mux].selectExp;
        bitNo = m_MuxAttributes[mux].selectBit;

        // Set the I/O Expander bit to the correct state.
        hr = _setExpBitToState(pin, expNo, bitNo, selection);
    }

    return hr;
}

/**
Method to set a GPIO pin to a specified state.
\param[in] pin The number of the pin in question.
\param[in] state The desired state (HIGH or LOW).
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::setPinState(ULONG pin, ULONG state)
{
    HRESULT hr = S_OK;

    if (state > 1)
    {
        hr = DMAP_E_INVALID_PIN_STATE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.setPinState(m_PinAttributes[pin].portBit, state);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.setS0PinState(m_PinAttributes[pin].portBit, state);
        case GPIO_S5:
            return g_btFabricGpio.setS5PinState(m_PinAttributes[pin].portBit, state);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

/**
Method to read a GPIO input pin.
\param[in] pin The number of the pin in question.
\param[out] state The variable to pass back the pin state (HIGH or LOW).
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::getPinState(ULONG pin, ULONG & state)
{
    HRESULT hr = S_OK;

    hr = _verifyBoardType();

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }
    
    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.getPinState(m_PinAttributes[pin].portBit, state);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.getS0PinState(m_PinAttributes[pin].portBit, state);
        case GPIO_S5:
            return g_btFabricGpio.getS5PinState(m_PinAttributes[pin].portBit, state);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

/**
This method expects the call to have verified the pin number is in range, supports
PWM functions, and is in PWM mode.
\param[in] pin The number of the GPIO pin in question.
\param[in] dutyCycle The desired duty-cycle of the positive pulses (0-0xFFFFFFFF for 0-100%).
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::setPwmDutyCycle(ULONG pin, ULONG dutyCycle)
{
    HRESULT hr = S_OK;
    ULONG expNo;
    ULONG channel;
    ULONG expType;
    ULONG i2cAdr;


    hr = _verifyBoardType();

    if (SUCCEEDED(hr))
    {
        switch (m_boardType)
        {
        case BOARD_TYPE::MBM_IKA_LURE:
            expNo = m_PwmChannels[pin].expander;
            channel = m_PwmChannels[pin].channel;
            expType = m_ExpAttributes[expNo].Exp_Type;
            i2cAdr = m_ExpAttributes[expNo].I2c_Address;

            // Dispatch to the correct code based on the PWM chip type:
            switch (expType)
            {
            case PCA9685:
                hr = PCA9685Device::SetPwmDutyCycle(i2cAdr, channel, dutyCycle);
                break;
            default:
                hr = DMAP_E_DMAP_INTERNAL_ERROR;
            }
            break;

        case BOARD_TYPE::MBM_BARE:
        case BOARD_TYPE::PI2_BARE:
            // Conver the pseudo-pin number passed in to a channel number.
            channel = pin - PWM0;

            // If we have a PWM chip, it is external to the board.  The only one we
            // currently support is the PCA9685.  Assume that is what we are using.
            hr = PCA9685Device::SetPwmDutyCycle(EXT_PCA9685_I2C_ADR, channel, dutyCycle);
            break;

        default:
            hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    return hr;
}


/**
This method expects the call to have verified the pin number is in range, supports
PWM functions, and is in PWM mode.
\param[in] pin The number of the GPIO pin in question.
\param[in] frequency The desired PWM pulse repetition frequency in Hz.
\return HRESULT success or error code.
\note The actual frequency set may be different than the frequency requested because
of hardware limitations and clock uncertainty.  Use getActualPwmFrequency() to get the 
approximate frequency that was actually set.
*/
HRESULT BoardPinsClass::setPwmFrequency(ULONG pin, ULONG frequency)
{
    HRESULT hr = S_OK;
    ULONG expNo;
    ULONG expType;
    ULONG i2cAdr;


    hr = _verifyBoardType();

    if (SUCCEEDED(hr))
    {
        switch (m_boardType)
        {
        case BOARD_TYPE::MBM_IKA_LURE:
            expNo = m_PwmChannels[pin].expander;
            expType = m_ExpAttributes[expNo].Exp_Type;
            i2cAdr = m_ExpAttributes[expNo].I2c_Address;

            // Dispatch to the correct code based on the PWM chip type:
            switch (expType)
            {
            case PCA9685:
                hr = PCA9685Device::SetPwmFrequency(i2cAdr, frequency);
                break;
            default:
                hr = DMAP_E_DMAP_INTERNAL_ERROR;
            }
            break;

        case BOARD_TYPE::MBM_BARE:
        case BOARD_TYPE::PI2_BARE:

            // If we have a PWM chip, it is external to the board.  The only one we
            // currently support is the PCA9685.  Assume that is what we are using.
            hr = PCA9685Device::SetPwmFrequency(EXT_PCA9685_I2C_ADR, frequency);
            break;

        default:
            hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
        }
    }

    return hr;
}

/**
This method expects the call to have verified the pin number is in range, supports
PWM functions, and is in PWM mode.
\param[in] pin The number of the GPIO pin in question.
\return ULONG the approximate value of the actual PWM pulse repetition rate in Hz.
*/
ULONG BoardPinsClass::getActualPwmFrequency(ULONG pin)
{
    HRESULT hr = S_OK;
    ULONG expNo;
    ULONG channel;
    ULONG expType;
    ULONG i2cAdr;
    ULONG pwmFrequency = 0;


    hr = _verifyBoardType();

    if (SUCCEEDED(hr))
    {
        switch (m_boardType)
        {
        case BOARD_TYPE::MBM_IKA_LURE:
            expNo = m_PwmChannels[pin].expander;
            channel = m_PwmChannels[pin].channel;
            expType = m_ExpAttributes[expNo].Exp_Type;
            i2cAdr = m_ExpAttributes[expNo].I2c_Address;

            // Dispatch to the correct code based on the PWM chip type:
            switch (expType)
            {
            case PCA9685:
                pwmFrequency = PCA9685Device::GetActualPwmFrequency(i2cAdr);
                break;
            }
            break;

        case BOARD_TYPE::MBM_BARE:
        case BOARD_TYPE::PI2_BARE:
            // If we have a PWM chip, it is external to the board.  The only one we
            // currently support is the PCA9685.  Assume that is what we are using.
            pwmFrequency = PCA9685Device::GetActualPwmFrequency(EXT_PCA9685_I2C_ADR);
            break;
        }
    }

    return pwmFrequency;
}

/**
The board type is determined by parsing the processor identifier string 
in the Registry.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_determineBoardType()
{
    HRESULT hr = S_OK;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:

    // For now, assume MBM for UWP x86 builds.
#if defined(_M_IX86) || defined(_M_X64)
    hr = _determineMbmConfig();
#endif // defined(_M_IX86) || defined(_M_X64)

    // For now, assume PI2 for UWP ARM builds.
#if defined(_M_ARM)
    hr = _determinePi2Config();
#endif // defined(_M_IX86) || defined(_M_X64)

#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
    HKEY baseKey = HKEY_LOCAL_MACHINE;
    HKEY regKey = nullptr;
    WCHAR subKey[] = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
    WCHAR valueName[] = L"Identifier";
    PWCHAR value = NULL;
    ULONG valueBytes = 0;
    WCHAR mbmId[] = L"Intel64 Family 6";

    error = RegOpenKeyEx(baseKey, subKey, 0, KEY_READ, &regKey);
    if (error != ERROR_SUCCESS) { hr = HRESULT_FROM_WIN32(error); }

    if (SUCCEEDED(hr))
    {
        error = RegGetValue(regKey, NULL, valueName, RRF_RT_REG_SZ, NULL, NULL, &valueBytes);
        if (error != ERROR_SUCCESS) { hr = HRESULT_FROM_WIN32(error); }

        if (SUCCEEDED(hr))
        {
            value = (PWCHAR) new UCHAR[valueBytes];
            if (value == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            error = RegGetValue(regKey, NULL, valueName, RRF_RT_REG_SZ, NULL, value, &valueBytes);
            if (error != ERROR_SUCCESS) { hr = HRESULT_FROM_WIN32(error); }

            if (SUCCEEDED(hr))
            {
                if ((wcslen(mbmId) < (valueBytes / sizeof(WCHAR))) && (wcsncmp(value, mbmId, wcslen(mbmId)) == 0))
                {
                    hr = _determineMbmConfig();
                }
                else
                {
                    m_boardType = NOT_SET;
                    hr = DMAP_E_BOARD_TYPE_NOT_RECOGNIZED;
                }
            }

            delete[] value;
        }

        RegCloseKey(regKey);
        regKey = nullptr;
    }
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    return hr;
}


/**
The Ika Lure has a ADC chip at address 0x48.  If this I2C address responds,
this code assumes an Ika Lure is attached to this MBM.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_determineMbmConfig()
{
    HRESULT hr = S_OK;

    // Start with the assumption this is a bare MBM board.  This is done so the
    // I2C code will know how to configure the Bay Trail I2C controller so it can be used 
    // to determine if the Ika Lure is attached.
    hr = setBoardType(MBM_BARE);

    if (SUCCEEDED(hr))
    {
        hr = _testI2cAddress(MBM_IKA_LURE_ADC_ADR);

        if (SUCCEEDED(hr))
        {
            hr = setBoardType(MBM_IKA_LURE);
        }
        else
        {
            // Error return from _testI2cAddress() is expected if the devices does not exist.
            hr = S_OK;
        }
    }

    return hr;
}

/**
As of the first writing of this code, we only support the bare PI2 board.
This code needs to be expanded as hats are supported.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_determinePi2Config()
{
    HRESULT hr = S_OK;

    hr = setBoardType(PI2_BARE);

    return hr;
}

/**
Method to manually specify the board type.  This can be used to avoid
board type auto-detection, which could cause problems if the user has
connect I2C slaves at the addresses we examine to determine board type.
\param[in] gen The type of board to set (PI2_BARE, etc).
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::setBoardType(BOARD_TYPE board)
{
    HRESULT hr = S_OK;

    m_boardType = board;
#if defined(_M_IX86) || defined(_M_X64)
    if (board == MBM_BARE)
    {
        m_PinAttributes = g_MbmPinAttributes;
        m_MuxAttributes = g_MbmMuxAttributes;
        m_PwmChannels = g_MbmPwmChannels;
        m_GpioPinCount = NUM_MBM_PINS;
    }
    else if (board == MBM_IKA_LURE)
    {
        m_PinAttributes = g_MbmIkaPinAttributes;
        m_MuxAttributes = g_MbmIkaMuxAttributes;
        m_PwmChannels = g_MbmIkaPwmChannels;
        m_GpioPinCount = NUM_ARDUINO_PINS;
    }
#endif // defined(_M_IX86) || defined(_M_X64)

#if defined(_M_ARM)
    if (board == PI2_BARE)
    {
        m_PinAttributes = g_Pi2PinAttributes;
        m_MuxAttributes = g_Pi2MuxAttributes;
        m_PwmChannels = g_Pi2PwmChannels;
        m_GpioPinCount = NUM_PI2_PINS;
    }
#endif // defined(_M_ARM)

    else
    {
        m_boardType = NOT_SET;
        hr = DMAP_E_INVALID_BOARD_TYPE_SPECIFIED;
    }

    return hr;
}

/**
Attempt to access an I2C slave at a specified address to determine if the slave
is present or not.
\return HRESULT success or error code.
*/
HRESULT BoardPinsClass::_testI2cAddress(ULONG i2cAdr)
{
    HRESULT hr = S_OK;
    I2cTransactionClass trans;
    UCHAR buffer[1] = { 0 };

    hr = trans.setAddress(i2cAdr);

    if (SUCCEEDED(hr))
    {
        hr = trans.queueWrite(buffer, sizeof(buffer));
    }

    if (SUCCEEDED(hr))
    {
        hr = trans.execute(g_i2c.getController());
    }

    return hr;
}

/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
\return Success or failure code.
*/
HRESULT BoardPinsClass::attachInterrupt(uint8_t pin, std::function<void(void)> func, int mode)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.attachInterrupt(m_PinAttributes[pin].portBit, func, mode);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.attachS0Interrupt(pin, func, mode);
        case GPIO_S5:
            return g_btFabricGpio.attachS5Interrupt(pin, func, mode);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
\return Success or failure code.
*/
HRESULT BoardPinsClass::attachInterruptEx(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, int mode)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.attachInterruptEx(m_PinAttributes[pin].portBit, func, mode);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.attachS0InterruptEx(pin, func, mode);
        case GPIO_S5:
            return g_btFabricGpio.attachS5InterruptEx(pin, func, mode);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
\param[in] context Data to be passed to the caller.
\return Success or failure code.
*/
HRESULT BoardPinsClass::attachInterruptContext(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, void* context, int mode)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.attachInterruptContext(m_PinAttributes[pin].portBit, func, context, mode);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.attachS0InterruptContext(pin, func, context, mode);
        case GPIO_S5:
            return g_btFabricGpio.attachS5InterruptContext(pin, func, context, mode);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

/**
\param[in] pin The number of the board pin for which interrupts are to be detached.
\return Success or failure code.
*/
HRESULT BoardPinsClass::detachInterrupt(uint8_t pin)
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = _verifyBoardType();
    }

    if (SUCCEEDED(hr) && !pinNumberIsSafe(pin))
    {
        hr = DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD;
    }

    if (SUCCEEDED(hr))
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
#if defined(_M_ARM)
        case GPIO_BCM:
            return g_bcmGpio.detachInterrupt(m_PinAttributes[pin].portBit);
#endif // defined(_M_ARM)
#if defined(_M_IX86) || defined(_M_X64)
        case GPIO_S0:
            return g_btFabricGpio.detachS0Interrupt(pin);
        case GPIO_S5:
            return g_btFabricGpio.detachS5Interrupt(pin);
#endif // defined(_M_IX86) || defined(_M_X64)
        default:
            hr = DMAP_E_DMAP_INTERNAL_ERROR;
        }
    }

    return hr;
}

