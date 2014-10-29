/** \file networkserial.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.
 * See License.txt in the project root for license information.
 */

#pragma once
#include "Stream.h"

/// \brief Used for serial communication between the Arduino
/// board and networked devices.
class NetworkSerial : public Stream
{
private:
    SOCKET _listenSocket = INVALID_SOCKET;
    SOCKET _clientSocket = INVALID_SOCKET;

public:
    NetworkSerial();

    /// \brief Get the number of bytes (characters) available for reading
    /// from the serial port.
    /// \details
    ///   This is data that's already arrived and stored in the serial
    ///   receive buffer (which holds 64 bytes).
    /// \return The number of bytes available to read
    /// \see <a href="http://arduino.cc/en/Serial/Available" target="_blank">origin: Arduino::Serial::available</a>
    virtual int available(void);

    /// \brief Sets the data rate in bits per second (baud) for serial data transmission.
    /// \details
    ///   For communicating with the computer, use one of these rates: 300,
    ///   600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600,
    ///   or 115200. You can, however, specify other rates - for example,
    ///   to communicate over pins 0 and 1 with a component that requires a
    ///   particular baud rate.
    /// \param [in] speed In bits per second (baud)
    /// \param [in] config Sets data, parity, and stop bits
    /// \n Valid values are:
    /// \arg SERIAL_5N1
    /// \arg SERIAL_6N1
    /// \arg SERIAL_7N1
    /// \arg SERIAL_8N1 (the default)
    /// \arg SERIAL_5N2
    /// \arg SERIAL_6N2
    /// \arg SERIAL_7N2
    /// \arg SERIAL_8N2
    /// \arg SERIAL_5E1
    /// \arg SERIAL_6E1
    /// \arg SERIAL_7E1
    /// \arg SERIAL_8E1
    /// \arg SERIAL_5E2
    /// \arg SERIAL_6E2
    /// \arg SERIAL_7E2
    /// \arg SERIAL_8E2
    /// \arg SERIAL_5O1
    /// \arg SERIAL_6O1
    /// \arg SERIAL_7O1
    /// \arg SERIAL_8O1
    /// \arg SERIAL_5O2
    /// \arg SERIAL_6O2
    /// \arg SERIAL_7O2
    /// \arg SERIAL_8O2
    /// \see <a href="http://arduino.cc/en/Serial/Begin" target="_blank">origin: Arduino::Serial::begin</a>
    void begin(unsigned long);

    /// \brief Reads incoming serial data.
    /// \return The first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// \see <a href="http://arduino.cc/en/Serial/Read" target="_blank">origin: Arduino::Serial::read</a>
    virtual int read(void);

    /// \brief Writes binary data to the serial port.
    /// \details
    ///   This data is sent as a byte or series of bytes; to send the
    ///   characters representing the digits of a number
    /// \param [in] buffer An array to send as a series of bytes
    /// \param [in] length The length of the buffer
    /// \return The number of bytes written
    /// \see <a href="http://arduino.cc/en/Serial/Write" target="_blank">origin: Arduino::Serial::write</a>
    virtual size_t write(const uint8_t *buffer, size_t length);
};

extern NetworkSerial Serial;