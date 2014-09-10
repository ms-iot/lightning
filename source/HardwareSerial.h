// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once

// Application indicates they want network serial instead of hardware serial.
// Some libraries will directly include hardware serial, so we switch here instead of in arduino.h
#ifdef USE_NETWORKSERIAL
#include "NetworkSerial.h"
#else

#include <cstdint>
#include <string>

#include "Stream.h"

/// <summary>
///   Used for communication between the Arduino board and a computer
///   or other devices.
/// </summary>
/// <remarks>
///   All Arduino boards have at least one serial port (also
///   known as a UART or USART): Serial. It communicates on digital pins 0
///   (RX) and 1 (TX) as well as with the computer via USB. Thus, if you use
///   these functions, you cannot also use pins 0 and 1 for digital input or
///   output.
/// </remarks>
class HardwareSerial : public Stream
{
private:
    HANDLE _comHandle = INVALID_HANDLE_VALUE;
    BYTE _storage[64];
    DWORD _storageCount;
    DWORD _storageIndex;
    bool _storageUsed;
    const std::wstring _comPortName;
    unsigned long _timeout;

    HardwareSerial & operator= (HardwareSerial &);

public:
    enum SerialConfigs
    {
        SERIAL_5N1,
        SERIAL_6N1,
        SERIAL_7N1,
        SERIAL_8N1,
        SERIAL_5N2,
        SERIAL_6N2,
        SERIAL_7N2,
        SERIAL_8N2,
        SERIAL_5E1,
        SERIAL_6E1,
        SERIAL_7E1,
        SERIAL_8E1,
        SERIAL_5E2,
        SERIAL_6E2,
        SERIAL_7E2,
        SERIAL_8E2,
        SERIAL_5O1,
        SERIAL_6O1,
        SERIAL_7O1,
        SERIAL_8O1,
        SERIAL_5O2,
        SERIAL_6O2,
        SERIAL_7O2,
        SERIAL_8O2,
    };

    static DCB dcbArray[24];

    HardwareSerial(const std::wstring &comPort = L"\\\\.\\COM1");

    virtual ~HardwareSerial();

    // Implementation of Arduino HardwareSerial::operator bool
    /// <summary>
    ///   Indicates if the specified Serial port is ready.
    /// </summary>
    /// <returns>
    ///   boolean : returns true if the specified serial port is available.
    /// </returns>
    explicit operator bool(void);


    // Implementation of Arduino HardwareSerial::available
    /// <summary>
    ///   Get the number of bytes (characters) available for reading from
    ///   the serial port.
    /// </summary>
    /// <remarks>
    ///   This is data that's already arrived and stored in the serial
    ///   receive buffer (which holds 64 bytes).
    /// </remarks>
    /// <returns>
    ///   int : the number of bytes available to read
    /// </returns>
    virtual int available(void);


    // Implementation of Arduino HardwareSerial::begin
    /// <summary>
    ///   Sets the data rate in bits per second (baud) for serial data transmission.
    /// </summary>
    /// <remarks>
    ///   For communicating with the computer, use one of these rates: 300,
    ///   600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600,
    ///   or 115200. You can, however, specify other rates - for example,
    ///   to communicate over pins 0 and 1 with a component that requires a
    ///   particular baud rate.
    /// </remarks>
    /// <param name="speed">
    ///   long : in bits per second (baud)
    /// </param>
    /// <param name="[config]">
    ///   SerialConfig : sets data, parity, and stop bits.
    ///   Valid values are:
    ///   <list type="bullet">
    ///     <item><description>SERIAL_5N1</description></item>
    ///     <item><description>SERIAL_6N1</description></item>
    ///     <item><description>SERIAL_7N1</description></item>
    ///     <item><description>SERIAL_8N1 (the default)</description></item>
    ///     <item><description>SERIAL_5N2</description></item>
    ///     <item><description>SERIAL_6N2</description></item>
    ///     <item><description>SERIAL_7N2</description></item>
    ///     <item><description>SERIAL_8N2</description></item>
    ///     <item><description>SERIAL_5E1</description></item>
    ///     <item><description>SERIAL_6E1</description></item>
    ///     <item><description>SERIAL_7E1</description></item>
    ///     <item><description>SERIAL_8E1</description></item>
    ///     <item><description>SERIAL_5E2</description></item>
    ///     <item><description>SERIAL_6E2</description></item>
    ///     <item><description>SERIAL_7E2</description></item>
    ///     <item><description>SERIAL_8E2</description></item>
    ///     <item><description>SERIAL_5O1</description></item>
    ///     <item><description>SERIAL_6O1</description></item>
    ///     <item><description>SERIAL_7O1</description></item>
    ///     <item><description>SERIAL_8O1</description></item>
    ///     <item><description>SERIAL_5O2</description></item>
    ///     <item><description>SERIAL_6O2</description></item>
    ///     <item><description>SERIAL_7O2</description></item>
    ///     <item><description>SERIAL_8O2</description></item>
    ///   </list>
    /// </param>
    /// <returns>
    ///   nothing
    /// </returns>
    void begin(unsigned long baud, uint8_t config = SERIAL_8N1);


    // Implementation of Arduino HardwareSerial::end
    /// <summary>
    ///   Disables serial communication, allowing the RX and TX pins to be
    ///   used for general input and output.
    /// </summary>
    /// <remarks>
    ///    To re-enable serial communication, call Serial.begin().
    /// </remarks>
    /// <returns>
    ///   nothing
    /// </returns>
    void end(void);

    // Implementation of Arduino HardwareSerial::flush
    /// <summary>
    ///   Waits for the transmission of outgoing serial data to complete.
    /// </summary>
    /// <remarks>
    ///   Prior to Arduino 1.0, this instead removed any buffered
    ///   incoming serial data.
    /// </remarks>
    /// <returns>
    ///   nothing
    /// </returns>
    virtual void flush(void);

    // Implementation of Arduino HardwareSerial::peek
    /// <summary>
    ///   Returns the next byte (character) of incoming serial data
    ///   without removing it from the internal serial buffer.
    /// </summary>
    /// <remarks>
    ///   Successive calls to peek() will return the same character;
    ///   the character from the next call to read().
    /// </remarks>
    /// <returns>
    ///   int : the first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// </returns>
    virtual int peek(void);

    // Implementation of Arduino HardwareSerial::read
    /// <summary>
    ///   Reads incoming serial data.
    /// </summary>
    /// <returns>
    ///   int : the first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// </returns>
    virtual int read(void);

    // Implementation of Arduino Stream::setTimeout
    /// <summary>
    ///   Sets the maximum milliseconds to wait for serial data when using
    ///   Serial.parseFloat(), Serial.parseInt(), Serial.readBytesUntil()
    ///   or Serial.readBytes()
    /// </summary>
    /// <remarks>
    ///   Defaults to 1000 milliseconds.
    /// </remarks>
    /// <param name="milliseconds">
    ///   unsigned long : timeout duration in milliseconds
    /// </param>
    /// <returns>
    ///   nothing
    /// </returns>
    virtual void setTimeout(unsigned long timeout);


    // Implementation of Arduino HardwareSerial::write
    /// <summary>
    ///   Writes binary data to the serial port.
    /// </summary>
    /// <remarks>
    ///   This data is sent as a byte or series of bytes; to send the
    ///   characters representing the digits of a number <see cref="HardwareSerial::print" />.
    /// </remarks>
    /// <param name="buffer">
    ///   uint8_t * : an array to send as a series of bytes
    /// </param>
    /// <param name="length">
    ///   size_t : the length of the buffer
    /// </param>
    /// <returns>
    ///   size_t : returns the number of bytes written
    /// </returns>
    virtual size_t write(uint8_t c);
    virtual size_t write(const uint8_t *buffer, size_t size);

    inline size_t write(unsigned long n)
    {
        // at least 32 bits in size
        return write((const uint8_t *) (&n), sizeof(unsigned long));
    }
    inline size_t write(long n)
    {
        // at least 32 bits in size
        return write((const uint8_t *) (&n), sizeof(long));
    }
    inline size_t write(unsigned int n)
    {
        // at least 16 bits in size
        return write((const uint8_t *) (&n), sizeof(unsigned int));
    }
    inline size_t write(int n)
    {
        // at least 16 bits in size
        return write((const uint8_t *) (&n), sizeof(int));
    }
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
#endif
