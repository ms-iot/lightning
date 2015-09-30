// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "EEPROM.h"
#include "wire.h"

// I2C transaction code(s):
namespace {
    const uint8_t EEPROM_ADDR7 = 0x50;
}

uint8_t
EEPROMClass::read (
    const int address
) const {
    // Request data from EEPROM
    Wire.begin();
    Wire.beginTransmission(EEPROM_ADDR7);
    Wire.write(static_cast<uint8_t>(address >> 8));  // high-byte
    Wire.write(static_cast<uint8_t>(address));  // low-byte
    if ( TwoWire::ADDR_NACK_RECV == Wire.endTransmission(false) ) { return 0; }

    // Return response
    Wire.requestFrom(EEPROM_ADDR7, 1);
    return static_cast<uint8_t>(Wire.read());
}

void
EEPROMClass::write (
    const int address,
    const uint8_t value
) const {
    // Write data to EEPROM
    Wire.begin();
    Wire.beginTransmission(EEPROM_ADDR7);
    Wire.write(static_cast<uint8_t>(address >> 8));  // high-byte
    Wire.write(static_cast<uint8_t>(address));  // low-byte
    Wire.write(value);
    Wire.endTransmission(true);
}

EEPROMClass EEPROM;
