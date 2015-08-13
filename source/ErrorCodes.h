// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef _ERROR_CODES_H_
#define _ERROR_CODES_H_

//
// Locally defined error HRESULT codes.
// All these codes use FACILITY_ITF and an error Code value >0x200.  This guarentees
// they won't conflict with Windows HRESULTS for other facilities, or with COM HRESULTS.
//

/// A pin is already locked for use for a function that conflicts with the use requested.
#define DMAP_E_PIN_FUNCTION_LOCKED 0x80040201

/// A pin number was specified that is beyond the range of pins supported by the board.
#define DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD 0x80040202

/// A function has been requested on a pin that does not support that function.
#define DMAP_E_FUNCTION_NOT_SUPPORTED_ON_PIN 0x80040203

/// A pin direction was specified that was neither INPUT nor OUPUT.
#define DMAP_E_INVALID_PIN_DIRECTION 0x80040204

/// An internal inconsistency in the DMap code has been found.
#define DMAP_E_DMAP_INTERNAL_ERROR 0x80040205

/// A desited state for a pin was specified that was neither HIGH nor LOW.
#define DMAP_E_INVALID_PIN_STATE_SPECIFIED 0x80040206

/// The board type could not be determined.
#define DMAP_E_BOARD_TYPE_NOT_RECOGNIZED 0x80040207

/// An invalid board type was specified.
#define DMAP_E_INVALID_BOARD_TYPE_SPECIFIED 0x80040208

/// An invalid handle was specified attempting to get a controller lock.
#define DMAP_E_I2C_INVALID_LOCK_HANDLE_SPECIFIED 0x80040209

/// The specified I2C address is outside the legal range for 7-bit I2C addresses.
#define DMAP_E_I2C_ADDRESS_OUT_OF_RANGE 0x8004020A

/// No, or empty, write buffer was specified.
#define DMAP_E_NO_OR_EMPTY_WRITE_BUFFER 0x8004020B

/// No, or zero length, read buffer was specified.
#define DMAP_E_NO_OR_ZERO_LENGTH_READ_BUFFER 0x8004020C

/// No callback routine was specified to be queued.
#define DMAP_E_NO_CALLBACK_ROUTINE_SPECIFIED 0x8004020D

/// More than 5 seconds elapsed waiting to acquire the I2C bus lock.
#define DMAP_E_I2C_BUS_LOCK_TIMEOUT 0x8004020E

/// Fewer than the expected number of bytes were received on the I2C bus.
#define DMAP_E_I2C_READ_INCOMPLETE 0x8004020F

/// More than the expected number of bytes were received on the I2C bus.
#define DMAP_E_I2C_EXTRA_DATA_RECEIVED 0x80040210

/// One or more transfers remained undone at the end of the I2C operation.
#define DMAP_E_I2C_OPERATION_INCOMPLETE 0x80040211

/// ADC data for a different channel than requested was received.
/**
This error is likely to indicate that two or more threads or proccesses are 
attempting to use the ADC at the same time.
*/
#define DMAP_E_ADC_DATA_FROM_WRONG_CHANNEL 0x80040212

/// The width of data sent does not match the data width set on the SPI controller.
/**
The SPI data width must be set before begin() is called since the controller
data width can't be changed while the controller is running.  Call SPI.end(), 
set the desired width, then call SPI.begin() to start the contrller again.
*/
#define DMAP_E_SPI_DATA_WIDTH_MISMATCH 0x80040213

/// The ADC does not have the channel that has been requested.
#define DMAP_E_ADC_DOES_NOT_HAVE_REQUESTED_CHANNEL 0x80040214

/// The specified BUS number does not exist on this board.
#define DMAP_E_SPI_BUS_REQUESTED_DOES_NOT_EXIST 0x80040215

/// The SPI mode specified is not a legal SPI mode value (0-3).
#define DMAP_E_SPI_MODE_SPECIFIED_IS_INVALID 0x80042016

/// The SPI speed specified is not in the supported range.
#define DMAP_E_SPI_SPEED_SPECIFIED_IS_INVALID 0x80040217

/// This SPI implementation does not support buffer transfers.
#define DMAP_E_SPI_BUFFER_TRANSFER_NOT_IMPLEMENTED 0x80040218

/// The port/bit specified does not exist on the device.
#define DMAP_E_INVALID_PORT_BIT_FOR_DEVICE 0x80040219

/// A GPIO operation was performed on a pin configured as a PWM output.
#define DMAP_E_GPIO_PIN_IS_SET_TO_PWM 0x8004021A

/// The specified number of bits per transfer is not supported by the SPI controller.
#define DMAP_E_SPI_DATA_WIDTH_SPECIFIED_IS_INVALID 0x8004021B

/// An attempt was made to map more than the maximum number of devices.
#define DMAP_E_TOO_MANY_DEVICES_MAPPED 0x8004021C

/// The specified device was not found on the systems.
#define DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM 0x8004021D

#endif  // _ERROR_CODES_H_