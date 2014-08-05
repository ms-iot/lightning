// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#pragma once

// Application indicates they want network serial instead of hardware serial.
// Some libraries will directly include hardware serial, so we switch here instead of in arduino.h
#ifdef USE_NETWORKSERIAL
#include "NetworkSerial.h"
#else

#include "stdint.h"
#include "stdlib.h"
#include <functional>
#include <string>
#include "Stream.h"


enum base_t : int {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16,
};

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
    enum serial_config_t : uint8_t {
        SERIAL_5N1 = 0x00,
        SERIAL_6N1 = 0x02,
        SERIAL_7N1 = 0x04,
        SERIAL_8N1 = 0x06,
        SERIAL_5N2 = 0x08,
        SERIAL_6N2 = 0x0A,
        SERIAL_7N2 = 0x0C,
        SERIAL_8N2 = 0x0E,
        SERIAL_5E1 = 0x20,
        SERIAL_6E1 = 0x22,
        SERIAL_7E1 = 0x24,
        SERIAL_8E1 = 0x26,
        SERIAL_5E2 = 0x28,
        SERIAL_6E2 = 0x2A,
        SERIAL_7E2 = 0x2C,
        SERIAL_8E2 = 0x2E,
        SERIAL_5O1 = 0x30,
        SERIAL_6O1 = 0x32,
        SERIAL_7O1 = 0x34,
        SERIAL_8O1 = 0x36,
        SERIAL_5O2 = 0x38,
        SERIAL_6O2 = 0x3A,
        SERIAL_7O2 = 0x3C,
        SERIAL_8O2 = 0x3E,
    };

public:
    HardwareSerial() { }


    virtual ~HardwareSerial()
    {
        //this->end();
    }


    // Implementation of Arduino HardwareSerial::operator bool
    /// <summary>
    ///   Indicates if the specified Serial port is ready.
    /// </summary>
    /// <returns>
    ///   boolean : returns true if the specified serial port is available.
    /// </returns>
    inline explicit operator bool(void) { return false; }


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
    virtual int available(void) { return 0; }


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
    void begin(unsigned long baud_, uint8_t config_ = SERIAL_8N1) {}


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
    void end(void) {}


    // Implementation of Arduino Stream::find
    /// <summary>
    ///   Reads data from the serial buffer until the target string of
    ///   given length is found.
    ///   <seealso cref="HardwareSerial::setTimeout" />
    /// </summary>
    /// <param name="target">
    ///   char * : the string to search for
    /// </param>
    /// <returns>
    ///   boolean : returns true if target string is found, false if it
    ///   times out.
    /// </returns>
    inline bool find(char *target_) {
        return false;
        //return findUntil(target_, NULL);
    }


    // Implementation of Arduino Stream::findUntil
    /// <summary>
    ///   Reads data from the serial buffer until a target string of
    ///   given length or terminator string is found.
    ///   <seealso cref="HardwareSerial::setTimeout" />
    /// </summary>
    /// <param name="target">
    ///   char * : the string to search for
    /// </param>
    /// <param name="terminal">
    ///   char * : the terminal string in the search
    /// </param>
    /// <returns>
    ///   boolean : returns true if target string is found, false if it
    ///   times out.
    /// </returns>
    bool findUntil(char *target_, char *terminal_) { return false; }


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
    virtual void flush(void) {}


    // Implementation of Arduino Stream::parseFloat
    /// <summary>
    ///   Returns the first valid floating point number from the Serial
    ///   buffer.
    /// </summary>
    /// <remarks>
    ///   Characters that are not digits (or the minus sign) are skipped.
    ///   parseFloat() is terminated by the first character that is not a
    ///   floating point number.
    /// </remarks>
    /// <returns>
    ///   float : the next valid float (or zero on timeout)
    /// </returns>
    float parseFloat(void) { return 0; }


    // Implementation of Arduino Stream::parseInt
    /// <summary>
    ///   Looks for the next valid integer in the incoming serial stream.
    ///   <seealso cref="HardwareSerial::setTimeout" />
    /// </summary>
    /// <remarks>
    ///   If no valid integer is found within one second (adjustable
    ///   through Serial.setTimeout() ) a default value of 0 will be
    ///   returned.
    /// </remarks>
    /// <returns>
    ///   int : the next valid integer (or zero on timeout)
    /// </returns>
    int parseInt(void) { return 0; }


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
    virtual int peek(void) { return 0; }


    // Implementation of Arduino Print::print
    /// <summary>
    ///   Prints data to the serial port as human-readable ASCII text.
    /// </summary>
    /// <remarks>
    ///   This command can take many forms. Numbers are printed using an
    ///   ASCII character for each digit. Floats are similarly printed as
    ///   ASCII digits, defaulting to two decimal places. Bytes are sent
    ///   as a single character. Characters and strings are sent as is.
    ///   For example:
    ///   <list type="bullet">
    ///     <item><description>Serial.print(78) gives "78"</description></item>
    ///     <item><description>Serial.print(1.23456) gives "1.23"</description></item>
    ///     <item><description>Serial.print('N') gives "N"</description></item>
    ///     <item><description>-Serial.print("Hello world.") gives "Hello world."</description></item>
    ///   </list>
    ///   An optional second parameter specifies the base (format) to use;
    ///   permitted values are BIN (binary, or base 2), OCT (octal, or
    ///   base 8), DEC (decimal, or base 10), HEX (hexadecimal, or base
    ///   16). For floating point numbers, this parameter specifies the
    ///   number of decimal places to use.
    ///   For example:
    ///   <list type="bullet">
    ///     <item><description>Serial.print(78, BIN) gives "1001110"</description></item>
    ///     <item><description>Serial.print(78, OCT) gives "116" </description></item>
    ///     <item><description>Serial.print(78, DEC) gives "78"</description></item>
    ///     <item><description>Serial.print(78, HEX) gives "4E"</description></item>
    ///     <item><description>Serial.print(1.23456, 0) gives "1"</description></item>
    ///     <item><description>Serial.print(1.23456, 2) gives "1.23"</description></item>
    ///     <item><description>Serial.print(1.23456, 4) gives "1.2346"</description></item>
    ///   </list>
    ///   You can pass flash-memory based strings to Serial.print() by
    ///   wrapping them with F().
    ///   For example:
    ///   <list type="bullet">
    ///     <item><description>Serial.print(F(“Hello World”))</description></item>
    ///   </list>
    ///   To send a single byte, <see cref="HardwareSerial::write" />.
    /// </remarks>
    /// <param name="val">
    ///   any data type : the value to print
    /// </param>
    /// <param name="[base|precision]">
    ///   int : specifies the number base (for integral data types) or
    ///   number of decimal places (for floating point types)
    /// </param>
    /// <returns>
    ///   size_t : returns the number of bytes written
    /// </returns>
    size_t print(const char val_[]) { return 0; }
    size_t print(char val_) { return 0; }
    size_t print(unsigned char val_, int base_ = DEC) { return 0; }
    size_t print(int val_, int base_ = DEC) { return 0; }
    size_t print(unsigned int val_, int base_ = DEC) { return 0; }
    size_t print(long val_, int base_ = DEC) { return 0; }
    size_t print(unsigned long val_, int base_ = DEC) { return 0; }
    size_t print(double val_, int precision_ = 2) { return 0; }
    size_t print(const std::wstring &val_) { return 0; }
    //TODO:size_t print(const Printable&) { return 0; }
    //TODO:size_t print(const __FlashStringHelper *) { return 0; }


    // Implementation of Arduino Print::println
    /// <summary>
    ///   Prints data to the serial port as human-readable ASCII text
    ///   followed by a carriage return character (ASCII 13, or '\r')
    ///   and a newline character (ASCII 10, or '\n').
    ///   <seealso cref="HardwareSerial::print" />
    /// </summary>
    /// <remarks>
    ///   This command takes the same forms as Serial.print().
    /// </remarks>
    /// <param name="val">
    ///   any data type : the value to print
    /// </param>
    /// <param name="format">
    ///   int : specifies the number base (for integral data types) or
    ///   number of decimal places (for floating point types)
    /// </param>
    /// <returns>
    ///   size_t : print() returns the number of bytes written, though
    ///   reading that number is optional
    /// </returns>
    size_t println(const char val_[]) { return 0; }
    size_t println(char val_) { return 0; }
    size_t println(unsigned char val_, int base_ = DEC) { return 0; }
    size_t println(int val_, int base_ = DEC) { return 0; }
    size_t println(unsigned int val_, int base_ = DEC) { return 0; }
    size_t println(long val_, int base_ = DEC) { return 0; }
    size_t println(unsigned long val_, int base_ = DEC) { return 0; }
    size_t println(double val_, int precision_ = 2) { return 0; }
    size_t println(const std::wstring &val_) { return 0; }
    size_t println(void) { return 0; }
    //TODO:size_t println(const Printable&) { return 0; }
    //TODO:size_t println(const __FlashStringHelper *) { return 0; }


    // Implementation of Arduino HardwareSerial::read
    /// <summary>
    ///   Reads incoming serial data.
    /// </summary>
    /// <returns>
    ///   int : the first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// </returns>
    virtual int read(void) { return 0; }


    // Implementation of Arduino Stream::readBytes
    /// <summary>
    ///   Reads characters from the serial port into a buffer.
    ///   <seealso cref="HardwareSerial::setTimeout" />
    /// </summary>
    /// <remarks>
    ///   The function terminates if the determined length has been
    ///   read or it times out.
    /// </remarks>
    /// <param name="buffer">
    ///   char * : the buffer to store the bytes in
    /// </param>
    /// <param name="length">
    ///   int : the number of bytes to read
    /// </param>
    /// <returns>
    ///   size_t : returns the number of characters placed in the buffer
    /// </returns>
    size_t readBytes(char *buffer_, size_t length_) { return 0; }


    // Implementation of Arduino Stream::readBytesUntil
    /// <summary>
    ///   Reads characters from the serial buffer into an array.
    ///   <seealso cref="HardwareSerial::setTimeout" />
    /// </summary>
    /// <remarks>
    ///   The function terminates if the terminator character is
    ///   detected, the determined length has been read, or it times out.
    /// </remarks>
    /// <param name="character">
    ///   char : the character to search for
    /// </param>
    /// <param name="buffer">
    ///   char * : the buffer to store the bytes in
    /// </param>
    /// <param name="length">
    ///   int : the number of bytes to read
    /// </param>
    /// <returns>
    ///   size_t : returns the number of characters placed in the buffer
    /// </returns>
    size_t readBytesUntil(char terminator_, char *buffer_, size_t length_) { return 0; }

    
    // Implementation of Arduino HardwareSerial::serialEvent
    /// <summary>
    ///   Called when data is available.
    /// </summary>
    /// <remarks>
    ///   Use Serial.read() to capture this data.
    /// </remarks>
    /// <param name="callback">
    ///   a callback function to invoke when serial data is available
    /// </param>
    /// <returns>
    ///   nothing
    /// </returns>
    void serialEvent(std::function<void(void)> callback_) {
        //_serial_data_callback = callback_;
    }

    
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
    void setTimeout(unsigned long milliseconds_) {}


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
    virtual size_t write(const uint8_t *buffer, size_t size) { return 0; }
    size_t write(const char *str) { return 0; }
    virtual size_t write(uint8_t c_) { return 0; }
    inline size_t write(unsigned long n_) { return write(static_cast<uint8_t>(n_)); }
    inline size_t write(long n_) { return write(static_cast<uint8_t>(n_)); }
    inline size_t write(unsigned int n_) { return write(static_cast<uint8_t>(n_)); }
    inline size_t write(int n_) { return write(static_cast<uint8_t>(n_)); }


  private:
      std::function<void(void)> _serial_data_callback;
};

extern HardwareSerial Serial;
#endif
