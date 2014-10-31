// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include <Windows.h>
#include "GalileoPins.h"
#include "I2cController.h"

// GPIO type values.
const UCHAR GPIO_FABRIC = 1;    ///< GPIO is from the SOC Fabric sub-system (memory mapped)
const UCHAR GPIO_LEGRES = 2;    ///< GPIO is from the SOC Legacy sub-system that can resume from sleep
const UCHAR GPIO_LEGCOR = 3;    ///< GPIO is from the SOC Legacy sub-system that can not resume from sleep
const UCHAR GPIO_EXP1 = 4;      ///< GPIO is from a port bit on I/O Expander 1
const UCHAR GPIO_EXP2 = 5;      ///< GPIO is from a port bit on  I/O Expander 2
const UCHAR GPIO_CY8 = 6;       ///< GPIO is from a port on the CY8 I/O Expander (on Gen1)

// GPIO pin driver selection values.
const UCHAR GPIO_INPUT_DRIVER_SELECT = 1;   ///< Specify input circuitry should be enabled
const UCHAR GPIO_OUTPUT_DRIVER_SELECT = 0;  ///< Specify output driver should be enabled

// I/O Expander name values.
const UCHAR EXP0       =  0;    ///< Value specifies I/O Expander 0
const UCHAR EXP1       =  1;    ///< Value specifies I/O Expander 1
const UCHAR EXP2       =  2;    ///< Value specifies I/O Expander 2
const UCHAR PWM        =  3;    ///< Value specifies PWM used as I/O Expander
const UCHAR CY8        =  4;    ///< Value specifies CY8 I/O Expander (on Gen1)
const UCHAR NUM_IO_EXP =  5;    ///< The number of I/O Expanders present
const UCHAR NO_X       = 15;    ///< Value specifies that no I/O Expander is used

// I/O Expander types.
const UCHAR PCAL9535A = 0;      ///< I/O Expander chip used on Gen2
const UCHAR PCA9685 = 1;        ///< PWM chip used on Gen2
const UCHAR CY8C9540A = 2;      ///< I/O Expander/PWM chip used on Gen1
const UCHAR NUM_EXP_TYPSES = 2; ///< The number of I/O Expanders types present

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

// MUX name values for Gen2.
const UCHAR MUX0      =  0;     ///< Mux number 0
const UCHAR MUX1      =  1;     ///< Mux number 1
const UCHAR MUX2      =  2;     ///< Mux number 2
const UCHAR MUX3      =  3;     ///< Mux number 3
const UCHAR MUX4      =  4;     ///< Mux number 4
const UCHAR MUX5      =  5;     ///< Mux number 5
const UCHAR MUX6      =  6;     ///< Mux number 6
const UCHAR MUX7      =  7;     ///< Mux number 7
const UCHAR MUX8      =  8;     ///< Mux number 8
const UCHAR MUX9      =  9;     ///< Mux number 9
const UCHAR MUX10     = 10;     ///< Mux number 10
const UCHAR AMUX1     = 11;     ///< Dual analog mux number 1
const UCHAR AMUX2_1   = 12;     ///< Mux number 1 in dual mux package number 2
const UCHAR AMUX2_2   = 13;     ///< Mux number 2 in dual mux package number 2
const UCHAR NO_MUX    = 15;     ///< Value indicates no mux is present

// MUX name values for Gen1.
const UCHAR MUX_U1_1  =  0;     ///< Mux number 1 in package U1 
const UCHAR MUX_U1_2  =  1;     ///< Mux number 2 in package U1 
const UCHAR MUX_U2_1  =  2;     ///< Mux number 1 in package U2 
const UCHAR MUX_U2_2  =  3;     ///< Mux number 2 in package U2
const UCHAR MUX_U3_1  =  4;     ///< Mux number 1 in package U3
const UCHAR MUX_U3_2  =  5;     ///< Mux number 2 in package U3
const UCHAR MUX_U4_1  =  6;     ///< Mux number 1 in package U4
const UCHAR MUX_U4_2  =  7;     ///< Mux number 2 in package U4
const UCHAR MUX_U5_1  =  8;     ///< Mux number 1 in package U5
const UCHAR MUX_U5_2  =  9;     ///< Mux number 2 in package U5
const UCHAR MUX_U6_1  = 10;     ///< Mux number 1 in package U6
const UCHAR MUX_U6_2  = 11;     ///< Mux number 2 in package U6
const UCHAR MUX_U7    = 12;     ///< Both muxes in pack U7
const UCHAR MUX_U9_1  = 13;     ///< Mux number 1 in package U9
const UCHAR MUX_U9_2  = 14;     ///< Mux number 2 in package U9

// Maximum number of MUXes.
const UCHAR MAX_MUXES = 15;     ///< Maximum number of MUXes on a Gen1 or Gen2 board.

/// The global table of pin attributes for the Galileo Gen2 board.
/**
This table contains all the pin-specific attributes needed to configure and use an I/O pin.
It is indexed by pin number (0 to NUM_ARDUINO_PINS-1).
*/
const GalileoPinsClass::PORT_ATTRIBUTES g_Gen2PinAttributes[] =
{
    //gpioType           pullupExp   triStExp    muxA             Muxes (A,B) by function:      triStIn   Function_mask
    //             portBit     pullupBit   triStBit      muxB     Dio  Pwm  AnIn I2C  Spi  Ser     _pad
    { GPIO_FABRIC, 3,    EXP1, P0_1, EXP1, P0_0, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SER },            // D0
    { GPIO_FABRIC, 4,    EXP0, P1_5, EXP0, P1_4, MUX7,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 1,0, 1, 0, FUNC_DIO | FUNC_SER },            // D1
    { GPIO_FABRIC, 5,    EXP1, P0_3, EXP1, P0_2, MUX10,  NO_MUX,  1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SER },            // D2
    { GPIO_FABRIC, 6,    EXP0, P0_1, EXP0, P0_0, MUX0,   MUX9,    0,0, 1,0, 0,0, 0,0, 0,0, 0,1, 1, 0, FUNC_DIO | FUNC_PWM | FUNC_SER }, // D3
    { GPIO_LEGRES, 4,    EXP1, P0_5, EXP1, P0_4, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO },                       // D4
    { GPIO_LEGCOR, 0,    EXP0, P0_3, EXP0, P0_2, MUX1,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D5
    { GPIO_LEGCOR, 1,    EXP0, P0_5, EXP0, P0_4, MUX2,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D6
    { GPIO_EXP1,   P0_6, EXP1, P0_7, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D7
    { GPIO_EXP1,   P1_0, EXP1, P1_1, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D8
    { GPIO_LEGRES, 2,    EXP0, P0_7, EXP0, P0_6, MUX3,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D9
    { GPIO_FABRIC, 2,    EXP0, P1_3, EXP0, P1_2, MUX6,   NO_MUX,  0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_PWM },            // D10
    { GPIO_LEGRES, 3,    EXP0, P1_1, EXP0, P1_0, MUX4,   MUX5,    0,0, 1,0, 0,0, 0,0, 0,1, 0,0, 1, 0, FUNC_DIO | FUNC_PWM | FUNC_SPI }, // D11
    { GPIO_FABRIC, 7,    EXP1, P1_3, EXP1, P1_2, NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 1, 0, FUNC_DIO | FUNC_SPI },            // D12
    { GPIO_LEGRES, 5,    EXP0, P1_7, EXP0, P1_6, MUX8,   NO_MUX,  0,0, 0,0, 0,0, 0,0, 1,0, 0,0, 1, 0, FUNC_DIO | FUNC_SPI },            // D13
    { GPIO_EXP2,   P0_0, EXP2, P0_1, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A0
    { GPIO_EXP2,   P0_2, EXP2, P0_3, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A1
    { GPIO_EXP2,   P0_4, EXP2, P0_5, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A2
    { GPIO_EXP2,   P0_6, EXP2, P0_7, NO_X, 0,    NO_MUX, NO_MUX,  0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A3
    { GPIO_EXP2,   P1_0, EXP2, P1_1, NO_X, 0,    AMUX1,  AMUX2_1, 1,1, 0,0, 1,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }, // A4
    { GPIO_EXP2,   P1_2, EXP2, P1_3, NO_X, 0,    AMUX1,  AMUX2_2, 1,1, 0,0, 1,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }  // A5
};

/// The global table of mux attributes for the Galileo Gen2 board.
/**
This table contains the information needed to set each mux to a desired state.  It is indexed by
mux number and specifies which port bit of which I/O Expander drives the mux selection signal.
*/
const GalileoPinsClass::MUX_ATTRIBUTES g_Gen2MuxAttributes[MAX_MUXES] =
{
    { PWM,  LED0 },     ///< MUX0
    { PWM,  LED2 },     ///< MUX1
    { PWM,  LED4 },     ///< MUX2
    { PWM,  LED6 },     ///< MUX3
    { PWM,  LED8 },     ///< MUX4
    { EXP1, P1_4 },     ///< MUX5
    { PWM,  LED10 },    ///< MUX6
    { EXP1, P1_5 },     ///< MUX7
    { EXP1, P1_6 },     ///< MUX8
    { PWM,  LED12 },    ///< MUX9
    { PWM,  LED13 },    ///< MUX10
    { EXP2, P1_4 },     ///< AMUX1
    { PWM,  LED14 },    ///< AMUX2_1
    { PWM,  LED15 },    ///< AMUX2_2
    { NO_X, 0 }         ///< Not used on Gen2
};

/// The global table of PWM information for the Galileo Gen2 board.
/**
This table contains the information needed to drive the PWM channels.  It is indexed by the
Galileo GPIO pin number, and specifies the chip and port-bit that implements PWM for that pin.
*/
const GalileoPinsClass::PWM_CHANNEL g_Gen2PwmChannels[] =
{
    { NO_X, 0, 0, 0 },          ///< D0
    { NO_X, 0, 0, 0 },          ///< D1
    { NO_X, 0, 0, 0 },          ///< D2
    { PWM, LED1, LED1 , 0 },    ///< D3
    { NO_X, 0, 0, 0 },          ///< D4
    { PWM, LED3, LED3, 0 },     ///< D5
    { PWM, LED5, LED5, 0 },     ///< D6
    { NO_X, 0, 0, 0 },          ///< D7
    { NO_X, 0, 0, 0 },          ///< D8
    { PWM, LED7, LED7, 0 },     ///< D9
    { PWM, LED11, LED11, 0 },   ///< D10
    { PWM, LED9, LED9, 0 },     ///< D11
    { NO_X, 0, 0, 0 },          ///< D12
    { NO_X, 0, 0, 0 },          ///< D13
    { NO_X, 0, 0, 0 },          ///< A0
    { NO_X, 0, 0, 0 },          ///< A1
    { NO_X, 0, 0, 0 },          ///< A2
    { NO_X, 0, 0, 0 },          ///< A3
    { NO_X, 0, 0, 0 },          ///< A4
    { NO_X, 0, 0, 0 }           ///< A5
};

/// The global table of I/O Expander attributes for the Galileo board.
/**
This table contains the information needed to communicate with each I/O Expander chip.
It is indexed by Expander number and contains the type of chip used for that I/O Expander
and the address on the I2C bus the chip responds to.
*/
const GalileoPinsClass::EXP_ATTRIBUTES g_GenxExpAttributes[] =
{
    { PCAL9535A, 0x25 },    ///< EXP0 - Gen2
    { PCAL9535A, 0x26 },    ///< EXP1 - Gen2
    { PCAL9535A, 0x27 },    ///< EXP2 - Gen2
    { PCA9685,   0x47 },    ///< PWM - Gen2
    { CY8C9540A, 0x20 }     ///< CY8 - Gen1
};

/// The Gen2 I/O expander signature.
const ULONG g_gen2ExpSig = 0x0F;

/// The Gen1 I/O expander signature.
const ULONG g_gen1ExpSig = 0x10;

/// The global table of pin attributes for the Galileo Gen1 board.
/**
This table contains all the pin-specific attributes needed to configure and use an I/O pin.
It is indexed by pin number (0 to NUM_ARDUINO_PINS-1).
*/
const GalileoPinsClass::PORT_ATTRIBUTES g_Gen1PinAttributes[] =
{
    //gpioType           pullExp  triSExp  muxA              Muxes (A,B) by function:      triStIn   Function_mask
    //             portBit     pullBit  triSBit      muxB    Dio  Pwm  AnIn I2C  Spi  Ser     _pad
    { GPIO_CY8,    P4_6, NO_X, 0, NO_X, 0, MUX_U2_1, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_SER },            // D0
    { GPIO_CY8,    P4_7, NO_X, 0, NO_X, 0, MUX_U2_2, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_SER },            // D1
    { GPIO_FABRIC, 6,    NO_X, 0, NO_X, 0, MUX_U9_2, NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D2
    { GPIO_FABRIC, 7,    NO_X, 0, NO_X, 0, MUX_U9_1, NO_MUX, 0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D3
    { GPIO_CY8,    P1_4, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO },                       // D4
    { GPIO_CY8,    P0_1, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D5
    { GPIO_CY8,    P1_0, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D6
    { GPIO_CY8,    P1_3, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D7
    { GPIO_CY8,    P1_2, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D8
    { GPIO_CY8,    P0_3, NO_X, 0, NO_X, 0, NO_MUX,   NO_MUX, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D9
    { GPIO_FABRIC, 2,    NO_X, 0, NO_X, 0, MUX_U1_1, NO_MUX, 0,0, 1,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM },            // D10
    { GPIO_CY8,    P1_1, NO_X, 0, NO_X, 0, MUX_U1_2, NO_MUX, 1,0, 1,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_PWM | FUNC_SPI }, // D11
    { GPIO_CY8,    P3_2, NO_X, 0, NO_X, 0, MUX_U3_1, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_SPI },            // D12
    { GPIO_CY8,    P3_3, NO_X, 0, NO_X, 0, MUX_U3_2, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_SPI },            // D13
    { GPIO_CY8,    P4_0, NO_X, 0, NO_X, 0, MUX_U4_1, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A0
    { GPIO_CY8,    P4_1, NO_X, 0, NO_X, 0, MUX_U4_2, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A1
    { GPIO_CY8,    P4_2, NO_X, 0, NO_X, 0, MUX_U5_1, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A2
    { GPIO_CY8,    P4_3, NO_X, 0, NO_X, 0, MUX_U5_2, NO_MUX, 1,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN },            // A3
    { GPIO_CY8,    P4_4, NO_X, 0, NO_X, 0, MUX_U6_1, MUX_U7, 1,1, 0,0, 0,1, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }, // A4
    { GPIO_CY8,    P4_5, NO_X, 0, NO_X, 0, MUX_U6_2, MUX_U7, 1,1, 0,0, 0,1, 0,0, 0,0, 0,0, 0, 0, FUNC_DIO | FUNC_AIN | FUNC_I2C }  // A5
};

/// The global table of mux attributes for the Galileo Gen1 board.
/**
This table contains the information needed to set each mux to a desired state.  It is indexed by
mux number and specifies which port bit of which I/O Expander drives the mux selection signal.
*/
const GalileoPinsClass::MUX_ATTRIBUTES g_Gen1MuxAttributes[MAX_MUXES] =
{
    { CY8, P3_6 },      ///< MUX_U1_1
    { CY8, P3_7 },      ///< MUX_U1_2
    { CY8, P3_4 },      ///< MUX_U2_1
    { CY8, P3_5 },      ///< MUX_U2_2
    { CY8, P5_2 },      ///< MUX_U3_1
    { CY8, P5_3 },      ///< MUX_U3_2
    { CY8, P3_1 },      ///< MUX_U4_1
    { CY8, P3_0 },      ///< MUX_U4_2
    { CY8, P0_7 },      ///< MUX_U5_1
    { CY8, P0_6 },      ///< MUX_U5_2
    { CY8, P0_5 },      ///< MUX_U6_1
    { CY8, P0_4 },      ///< MUX_U6_2
    { CY8, P1_5 },      ///< MUX_U7
    { CY8, P1_6 },      ///< MUX_U9_1
    { CY8, P1_7 }       ///< MUX_U9_2
};

/// The global table of PWM information for the Galileo Gen1 board.
/**
This table contains the information needed to drive the PWM channels.  It is indexed by the
Galileo GPIO pin number, and specifies the chip and PWM channel used for that pin.
*/
const GalileoPinsClass::PWM_CHANNEL g_Gen1PwmChannels[] =
{
    { NO_X, 0, 0, 0 },        ///< D0
    { NO_X, 0, 0, 0 },        ///< D1
    { NO_X, 0, 0, 0 },        ///< D2
    { CY8, 3, P0_2, 0 },      ///< D3
    { NO_X, 0, 0, 0 },        ///< D4
    { CY8, 5, P0_1, 0 },      ///< D5
    { CY8, 6, P1_0, 0 },      ///< D6
    { CY8, 0, P1_3, 0 },      ///< D7
    { CY8, 2, P1_2, 0 },      ///< D8
    { CY8, 1, P0_3, 0 },      ///< D9
    { CY8, 7, P0_0, 0 },      ///< D10
    { CY8, 4, P1_1, 0 },      ///< D11
    { NO_X, 0, 0, 0 },        ///< D12
    { NO_X, 0, 0, 0 },        ///< D13
    { NO_X, 0, 0, 0 },        ///< A0
    { NO_X, 0, 0, 0 },        ///< A1
    { NO_X, 0, 0, 0 },        ///< A2
    { NO_X, 0, 0, 0 },        ///< A3
    { NO_X, 0, 0, 0 },        ///< A4
    { NO_X, 0, 0, 0 }         ///< A5
};

/// The global table of Pin Function tracking structures.
/**
This table tracks the currently configured function for each pin of the Galileo board.
*/
GalileoPinsClass::PIN_FUNCTION g_GenxPinFunctions[] = 
{
    { FUNC_NUL, false },    ///< D0
    { FUNC_NUL, false },    ///< D1
    { FUNC_NUL, false },    ///< D2
    { FUNC_NUL, false },    ///< D3
    { FUNC_NUL, false },    ///< D4
    { FUNC_NUL, false },    ///< D5
    { FUNC_NUL, false },    ///< D6
    { FUNC_NUL, false },    ///< D7
    { FUNC_NUL, false },    ///< D8
    { FUNC_NUL, false },    ///< D9
    { FUNC_NUL, false },    ///< D10
    { FUNC_NUL, false },    ///< D11
    { FUNC_NUL, false },    ///< D12
    { FUNC_NUL, false },    ///< D13
    { FUNC_NUL, false },    ///< A0
    { FUNC_NUL, false },    ///< A1
    { FUNC_NUL, false },    ///< A2
    { FUNC_NUL, false },    ///< A3
    { FUNC_NUL, false },    ///< A4
    { FUNC_NUL, false }     ///< A5
};

/// Constructor.
/**
Initialize the data members that point to the global attributes tables.
*/
GalileoPinsClass::GalileoPinsClass()
    :
    m_boardGeneration(0),
    m_PinAttributes(NULL),
    m_MuxAttributes(NULL),
    m_ExpAttributes(g_GenxExpAttributes),
    m_PinFunctions(g_GenxPinFunctions),
    m_PwmChannels(NULL)
{
}

/**
Method to determine whether a pin is set to a desired function or not, 
and to set it to that function if possible.
\param[in] pin The number of the pin in question
\param[in] function The desired function. See functions below.
\param[in] lockAction Desired lock action.  See lock actions below.
\return TRUE success. FALSE failure, GetLastError() provides error code.
\sa FUNC_DIO \sa FUNC_PWM \sa FUNC_AIN \sa FUNC_I2C \sa FUNC_SPI \sa FUNC_SER
\sa NO_LOCK_CHANGE \sa LOCK_FUNCTION \sa UNLOCK_FUNCTION
*/
BOOL GalileoPinsClass::verifyPinFunction(ULONG pin, ULONG function, FUNC_LOCK_ACTION lockAction)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (lockAction == UNLOCK_FUNCTION)
    {
        m_PinFunctions[pin].locked = false;
    }

    if (m_PinFunctions[pin].currentFunction != function)
    {
        if (m_PinFunctions[pin].locked)
        {
            status = FALSE;
            error = ERROR_LOCKED;
        }
        else
        {
            status = _setPinFunction(pin, function);
            if (!status) { error = GetLastError(); }

            if (status)
            {
                m_PinFunctions[pin].currentFunction = (UCHAR) function;
                if ((function == FUNC_SPI) || (function == FUNC_I2C))
                {
                    m_PinFunctions[pin].locked = true;
                }
            }
        }
    }

    if (status && (lockAction == LOCK_FUNCTION))
    {
        m_PinFunctions[pin].locked = true;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method prepares an external pin to be used for a specific function.
Functions are Digital I/O, Analog In, PWM, etc.
\param[in] pin the number of the pin in question.
\param[in] function the function to be used on the pin.
\return TRUE success. FALSE failure, GetLastError() provides error code.
\sa FUNC_DIO \sa FUNC_PWM \sa FUNC_AIN \sa FUNC_I2C \sa FUNC_SPI \sa FUNC_SER
*/
BOOL GalileoPinsClass::_setPinFunction(ULONG pin, ULONG function)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    // Verify the pin number is in range.
    status = _pinNumberIsValid(pin);
    if (!status) { error = ERROR_INVALID_PARAMETER; }

    if (status)
    {
        // Make sure the pin attributes table is set up for the board generation.
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    // Verify the requsted function is supported on this pin.
    if (status && ((m_PinAttributes[pin].funcMask & function) == 0))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        if (function == FUNC_DIO)
        {
            status = _setPinDigitalIo(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_PWM)
        {
            status = _setPinPwm(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_AIN)
        {
            status = _setPinAnalogInput(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_I2C)
        {
            status = _setPinI2c(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_SPI)
        {
            status = _setPinSpi(pin);
            if (!status) { error = GetLastError(); }
        }
        else if (function == FUNC_SER)
        {
            status = _setPinHwSerial(pin);
            if (!status) { error = GetLastError(); }
        }
        else
        {
            status = FALSE;
            ERROR_INVALID_PARAMETER;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method sets the current function of a pin to Digial I/O.  This code
assumes the caller has verified the pin number is in range and that Digital
I/O is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinDigitalIo(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at least one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Digital I/O.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].digIoMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Digital I/O.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].digIoMuxB);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is on a CY8C9540A I/O Expander chip, deselect the PWM function.
    if (status && (m_PwmChannels[pin].expander == CY8))
    {
        ULONG i2cAdr = m_ExpAttributes[m_PwmChannels[pin].expander].I2c_Address;
        ULONG portBit = m_PwmChannels[pin].portBit;
        status = CY8C9540ADevice::SetPortbitDio(i2cAdr, portBit);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the current function of a pin to PWM.  This code
assumes the caller has verified the pin number is in range and that PWM
is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinPwm(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for PWM.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].pwmMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for PWM.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].pwmMuxB);
        if (!status) { error = GetLastError(); }
    }

    // Configure the pin for driving a PWM signal.
    if (status)
    {
        if (m_PwmChannels[pin].expander == CY8)
        {
            // If PWM on this pin is from a CY8 I/O expander, configure the pin for PWM.
            ULONG i2cAdr = m_ExpAttributes[m_PwmChannels[pin].expander].I2c_Address;
            ULONG portBit = m_PwmChannels[pin].portBit;
            ULONG pwmChan = m_PwmChannels[pin].channel;
            status = CY8C9540ADevice::SetPortbitPwm(i2cAdr, portBit, pwmChan);
            if (!status) { error = GetLastError(); }
        }
        else
        {
            // If from a non-CY8 PWM chip, just set the pin to be an output.
            status = setPinMode(pin, DIRECTION_OUT, FALSE);
            if (!status) { error = GetLastError(); }
        }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the current function of a pin to Analog Input.  This code
assumes the caller has verified the pin number is in range and that Analog
Input is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinAnalogInput(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Analog Input.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].anInMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Analog Input.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].anInMuxB);
        if (!status) { error = GetLastError(); }
    }

    // Make sure the digital I/O functions on this pin are set to INPUT without pull-up.
    if (status)
    {
        status = setPinMode(pin, DIRECTION_IN, FALSE);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the current function of a pin to I2C Bus.  This code
assumes the caller has verified the pin number is in range and that I2C
bus use is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinI2c(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for I2C bus use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].i2cMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for I2C bus use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].i2cMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the current function of a pin to SPI Bus.  This code
assumes the caller has verified the pin number is in range and that SPI
bus use is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinSpi(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for SPI bus use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].spiMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for SPI bus use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].spiMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the current function of a pin to Hardware Serial.  This code
assumes the caller has verified the pin number is in range and that Hardware 
Serial is supported on the specified pin.
\param[in] pin The number of the pin in question.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setPinHwSerial(ULONG pin)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    // If the pin is tied to at lease one MUX:
    if (m_PinAttributes[pin].muxA != NO_MUX)
    {
        // Set the MUX to the desired state for Hardware Serial use.
        status = _setMux(m_PinAttributes[pin].muxA, m_PinAttributes[pin].serMuxA);
        if (!status) { error = GetLastError(); }
    }

    // If the pin is tied to a second MUX:
    if (status && (m_PinAttributes[pin].muxB != NO_MUX))
    {
        // Set the MUX to the desired state for Hardware Serial use.
        status = _setMux(m_PinAttributes[pin].muxB, m_PinAttributes[pin].serMuxB);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
This method sets the mode and drive type of a pin (Input, Output, etc.)
\param[in] pin The number of the pin in question.
\param[in] mode The desired mode: DIRECTION_IN or DIRECTION_OUT.
\param[in] pullup True to enable pin pullup resistor, false to disable pullup
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::setPinMode(ULONG pin, ULONG mode, BOOL pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;


    if (!_pinNumberIsValid(pin))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }
   
    if (status && (mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Set the pin direction on the device that supports this pin.
        switch (m_PinAttributes[pin].gpioType)
        {
        case GPIO_FABRIC:
            status = g_fabricGpio.setPinDirection(m_PinAttributes[pin].portBit, mode);
            if (!status) { error = GetLastError(); }
            break;
        case GPIO_LEGRES:
            status = g_legacyGpio.setResumePinDirection(m_PinAttributes[pin].portBit, mode);
            if (!status) { error = GetLastError(); }
            break;
        case GPIO_LEGCOR:
            status = g_legacyGpio.setCorePinDirection(m_PinAttributes[pin].portBit, mode);
            if (!status) { error = GetLastError(); }
            break;
        case GPIO_EXP1:
            status = _setExpBitDirection(EXP1, m_PinAttributes[pin].portBit, mode, pullup);
            if (!status) { error = GetLastError(); }
            break;
        case GPIO_EXP2:
            status = _setExpBitDirection(EXP2, m_PinAttributes[pin].portBit, mode, pullup);
            if (!status) { error = GetLastError(); }
            break;
        case GPIO_CY8:
            status = _setExpBitDirection(CY8, m_PinAttributes[pin].portBit, mode, pullup);
            if (!status) { error = GetLastError(); }
            break;
        default:
            status = FALSE;
            error = DNS_ERROR_INVALID_TYPE;
        }
    }

    if (status)
    {
        // Configure the pin drivers as needed.
        status = _configurePinDrivers(pin, mode);
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Configure the pin pullup as requested.
        status = _configurePinPullup(pin, pullup);
        if (!status) { error = GetLastError(); }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method sets the direction of an I/O Expander port pin.
\param[in] expNo The number of the I/O Expander in question.
\param[in] bitNo Specifies the port and bit number to set (such as: P1_4 for port 1, bit 4)
\param[in] direction The desired direction: DIRECTION_IN or DIRECTION_OUT.
\param[in] pullup TRUE - enable pullup resistor on the pin, FALSE - disable pullup resistor.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setExpBitDirection(ULONG expNo, ULONG bitNo, ULONG direction, BOOL pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR i2cAdr = 0;               // I2C address of I/O Expander


    // Get the I2C Address of the I/O Expander in question.
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    //
    // Determine what type of I/O Expander we are dealing with.
    //

    if (m_ExpAttributes[expNo].Exp_Type == PCAL9535A)
    {
        // Set the bit of the I/O Expander chip to the desired direction.
        status = PCAL9535ADevice::SetBitDirection(i2cAdr, bitNo, direction);
        if (!status) { error = GetLastError(); }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == CY8C9540A)
    {
        // Set the bit of the I/O Expander chip to the desired direction.
        status = CY8C9540ADevice::SetBitDirection(i2cAdr, bitNo, direction, pullup);
        if (!status) { error = GetLastError(); }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == PCA9685)
    {
        // Nothing to do here, PCA9685 chip ports are always outputs.
    }
    else
    {
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method sets the state of an I/O Expander port pin.
\param[in] expNo The number of the I/O Expander in question.
\param[in] bitNo Specifies the port and bit number to set (such as: P1_4 for port 1, bit 4)
\param[in] state The desired state: HIGH or LOW.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
// Method to set an I/O Expander bit to the specified state.
BOOL GalileoPinsClass::_setExpBitToState(ULONG expNo, ULONG bitNo, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR i2cAdr = 0;               // I2C address of I/O Expander


    // Get the I2C Address of the I/O Expander in question.
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    //
    // Determine what type of I/O Expander we are dealing with.
    //

    if (m_ExpAttributes[expNo].Exp_Type == PCAL9535A)
    {
        // Set the bit of the I/O Expander chip to the desired state.
        status = PCAL9535ADevice::SetBitState(i2cAdr, bitNo, state);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Set the bit of the I/O Expander chip to be an output.
            status = PCAL9535ADevice::SetBitDirection(i2cAdr, bitNo, DIRECTION_OUT);
            if (!status) { error = GetLastError(); }
        }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == PCA9685)
    {
        // Set the bit of the PWM chip to the desired state.
        status = PCA9685Device::SetBitState(i2cAdr, bitNo, state);
        if (!status) { error = GetLastError(); }
    }
    else if (m_ExpAttributes[expNo].Exp_Type == CY8C9540A)
    {
        // Set the bit of a CY8 I/O Expander chip to the desired state.
        status = CY8C9540ADevice::SetBitState(i2cAdr, bitNo, state);
        if (!status) { error = GetLastError(); }

        if (status)
        {
            // Set the bit of the I/O Expander chip to be an output.
            status = CY8C9540ADevice::SetBitDirection(i2cAdr, bitNo, DIRECTION_OUT, FALSE);
            if (!status) { error = GetLastError(); }
        }
    }
    else
    {
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to configure the pin input and output drivers to reflect the desired direction of a pin.
This code assumes the caller has verified the pin number and mode to be in range.
\param[in] pin The number of the pin in question.
\param[in] mode The desired direction for the pin.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_configurePinDrivers(ULONG pin, ULONG mode)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    UCHAR expNo = 0;                // I/O Expander number
    UCHAR bitNo = 0;                // Bit number on I/O Expander
    UCHAR state = 0;                // Desired pin state


    if ((mode != DIRECTION_IN) && (mode != DIRECTION_OUT))
    {
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    if (status)
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
            status = _setExpBitToState(expNo, bitNo, state);
            if (!status) { error = GetLastError(); }
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to configure the pin pullup as specified.  This code assumes the caller has
verified the pin number to be in the valid range.
\param[in] pin The number of the pin in question.
\param[in] pullup True to turn pullup on, false to turn pullup off.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_configurePinPullup(ULONG pin, BOOL pullup)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    ULONG expNo = 0;                // I/O Expander number
    ULONG bitNo = 0;                // Bit number on I/O Expander
    ULONG state = 0;                // Desired pin state

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Determine which I/O Expander pin controls the GPIO-pin drivers.
        expNo = m_PinAttributes[pin].pullupExp;
        bitNo = m_PinAttributes[pin].pullupBit;

        // If this pin has a pullup controlled by an I/O Expander port output pin:
        if (expNo != NO_X)
        {
            // If the pullup is wanted:
            if (pullup)
            {
                // Set the I/O Expander bit high (also sets it as an output)
                status = _setExpBitToState(expNo, bitNo, 1);
                if (!status)  { error = GetLastError(); }
            }

            // If no pullup is wanted:
            else
            {
                // Make the I/O Expander bit an input.
                status = _setExpBitDirection(expNo, bitNo, DIRECTION_IN, FALSE);
                if (!status)  { error = GetLastError(); }
            }
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to set a MUX to select the desired signal.
\param[in] mux The number of the MUX in question.
\param[in] selection The desired state of the MUX "select" input (LOW or HIGH).
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_setMux(ULONG mux, ULONG selection)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    ULONG expNo = 0;                // I/O Expander number
    ULONG bitNo = 0;                // Bit number on I/O Expander


    // If the MUX number is outside the valid range for this board, fail.
    if ((mux >= MAX_MUXES) || (m_MuxAttributes[mux].selectExp == NO_X))
    {
        status = FALSE;
        error = ERROR_INVALID_PARAMETER;
    }

    if (status)
    {
        // Determine which I/O Expander and pin drive the MUX select input.
        expNo = m_MuxAttributes[mux].selectExp;
        bitNo = m_MuxAttributes[mux].selectBit;

        // Set the I/O Expander bit to the correct state.
        status = _setExpBitToState(expNo, bitNo, selection);
        if (!status) { error = GetLastError(); }
    }

    if (!status)
    {
        SetLastError(error);
    }
    return status;
}

/**
Method to set a GPIO pin to a specified state.
\param[in] pin The number of the pin in question.
\param[in] state The desired state (HIGH or LOW).
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::setPinState(ULONG pin, ULONG state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (!_pinNumberIsValid(pin))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status && (state > 1))
    {
        status = FALSE;
        error = ERROR_INVALID_STATE;
    }

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
        case GPIO_FABRIC:
            return g_fabricGpio.setPinState(m_PinAttributes[pin].portBit, state);
        case GPIO_LEGRES:
            return g_legacyGpio.setResumePinState(m_PinAttributes[pin].portBit, state);
        case GPIO_LEGCOR:
            return g_legacyGpio.setCorePinState(m_PinAttributes[pin].portBit, state);
        case GPIO_EXP1:
            return PCAL9535ADevice::SetBitState(
                m_ExpAttributes[EXP1].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        case GPIO_EXP2:
            return PCAL9535ADevice::SetBitState(
                m_ExpAttributes[EXP2].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        case GPIO_CY8:
            return CY8C9540ADevice::SetBitState(
                m_ExpAttributes[CY8].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        default:
            status = FALSE;
            error = DNS_ERROR_INVALID_TYPE;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to read a GPIO input pin.
\param[in] pin The number of the pin in question.
\param[out] state The variable to pass back the pin state (HIGH or LOW).
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::getPinState(ULONG pin, ULONG & state)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (!_pinNumberIsValid(pin))
    {
        status = FALSE;
        error = ERROR_INVALID_ADDRESS;
    }

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    if (status)
    {
        // Dispatch to the correct method according to the type of GPIO pin we are dealing with.
        switch (m_PinAttributes[pin].gpioType)
        {
        case GPIO_FABRIC:
            return g_fabricGpio.getPinState(m_PinAttributes[pin].portBit, state);
        case GPIO_LEGRES:
            return g_legacyGpio.getResumePinState(m_PinAttributes[pin].portBit, state);
        case GPIO_LEGCOR:
            return g_legacyGpio.getCorePinState(m_PinAttributes[pin].portBit, state);
        case GPIO_EXP1:
            return PCAL9535ADevice::GetBitState(
                m_ExpAttributes[EXP1].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        case GPIO_EXP2:
            return PCAL9535ADevice::GetBitState(
                m_ExpAttributes[EXP2].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        case GPIO_CY8:
            return CY8C9540ADevice::GetBitState(
                m_ExpAttributes[CY8].I2c_Address,
                m_PinAttributes[pin].portBit,
                state);
        default:
            status = FALSE;
            error = DNS_ERROR_INVALID_TYPE;
        }
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method expects the call to have verified the pin number is in range, supports
PWM functions, and is in PWM mode.
\param[in] pin The number of the GPIO pin in question.
\param[in] dutyCycle The desired duty-cycle of the positive pulses (0-0xFFFFFFFF for 0-100%).
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::setPwmDutyCycle(ULONG pin, ULONG dutyCycle)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    ULONG expNo;
    ULONG channel;
    ULONG expType;
    ULONG i2cAdr;

    if (status)
    {
        status = _verifyBoardGeneration();
        if (!status) { error = GetLastError(); }
    }

    // These statements depend on the board generation being set, so they must come
    // after the _verifyBoardGeneration() call.
    expNo = m_PwmChannels[pin].expander;
    channel = m_PwmChannels[pin].channel;
    expType = m_ExpAttributes[expNo].Exp_Type;
    i2cAdr = m_ExpAttributes[expNo].I2c_Address;

    // Dispatch to the correct code based on the PWM chip type:
    switch (expType)
    {
    case PCA9685:
        status = PCA9685Device::SetPwmDutyCycle(i2cAdr, channel, dutyCycle);
        if (!status) { error = GetLastError(); }
        break;
    case CY8C9540A:
        status = CY8C9540ADevice::SetPwmDutyCycle(i2cAdr, channel, dutyCycle);
        if (!status) { error = GetLastError(); }
        break;
    default:
        status = FALSE;
        error = ERROR_NOT_SUPPORTED;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to determine the generation of the board this code is running on, and
to configure the code appropriately.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::_determineBoardGeneration()
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;
    ULONG expSig = 0;
    int i;

    // Try to access each of the known I/O Expander chips.
    for (i = 0; i < NUM_IO_EXP; i++)
    {
        if (_testI2cAddress(g_GenxExpAttributes[i].I2c_Address))
        {
            expSig = expSig | (1 << i);
        }
    }

    // Compare the signature of expanders found to the Gen1 and Gen2 signatures.
    if ((expSig & (g_gen2ExpSig)) == g_gen2ExpSig)
    {
        status = setBoardGeneration(2);
        if (!status) { error = GetLastError(); }
    }
    else if ((expSig & (g_gen1ExpSig)) == g_gen1ExpSig)
    {
        status = setBoardGeneration(1);
        if (!status) { error = GetLastError(); }
    }
    else
    {
        m_boardGeneration = 0;
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Method to manually specify the board generation.  This can be used to avoid
board generation auto-detection, which could cause problems if the user has
connect I2C slaves at the addresses of the I/O expanders.
\param[in] gen The generation of board to set (currently should be 1 or 2).
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::setBoardGeneration(ULONG gen)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    if (gen == 2)
    {
        m_boardGeneration = 2;
        m_PinAttributes = g_Gen2PinAttributes;
        m_MuxAttributes = g_Gen2MuxAttributes;
        m_PwmChannels = g_Gen2PwmChannels;
    }
    else if (gen == 1)
    {
        m_boardGeneration = 1;
        m_PinAttributes = g_Gen1PinAttributes;
        m_MuxAttributes = g_Gen1MuxAttributes;
        m_PwmChannels = g_Gen1PwmChannels;
    }
    else
    {
        m_boardGeneration = 0;
        status = FALSE;
        error = ERROR_INVALID_ENVIRONMENT;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
This method determines the board generation if it is not yet known.
\param[out] gen The generation of the board we are currently running on.
\return TRUE success. FALSE failure, GetLastError() provides error code.
*/
BOOL GalileoPinsClass::getBoardGeneration(ULONG & gen)
{
    BOOL status = TRUE;
    DWORD error = ERROR_SUCCESS;

    status = _verifyBoardGeneration();
    if (!status) { error = GetLastError(); }

    if (status)
    {
        gen = m_boardGeneration;
    }

    if (!status) { SetLastError(error); }
    return status;
}

/**
Attempt to access an I2C slave at a specified address to determine if the slave
is present or not.
\param[in] i2cAdr The I2C address to probe.
*/
BOOL GalileoPinsClass::_testI2cAddress(ULONG i2cAdr)
{
    BOOL status = TRUE;
    I2cTransactionClass trans;
    UCHAR buffer[1] = { 0 };

    status = trans.setAddress(i2cAdr);

    if (status)
    {
        status = trans.queueWrite(buffer, sizeof(buffer));
    }

    if (status)
    {
        status = trans.execute();
    }

    return status;
}



