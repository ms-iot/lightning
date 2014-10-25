/** \file hardwareserial.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.
 * See License.txt in the project root for license information.
 */

#pragma once

/// /brief Switch for network serial instead of hardware serial.
/// /details When the application indicates it wants network serial
/// instead of hardware serial. Some libraries will directly include
/// hardware serial, so we switch here instead of in arduino.h
#ifdef USE_NETWORKSERIAL
#include "NetworkSerial.h"
#else

#include <cstdint>
#include <string>

#include "Stream.h"

/// \brief Used for communication between the Arduino board and a computer
/// or other devices.
/// \details
///   All Arduino boards have at least one serial port (also known as a UART
///   or USART): Serial. It communicates on digital pins 0 (RX) and 1 (TX)
///   as well as with the computer via USB. Thus, if you use these functions,
///   you cannot also use pins 0 and 1 for digital input or output.
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

    /// \brief Hardware serial constructor
    /// \param [in] [comPort] Specify which communication port to
    /// send HardwareSerial transmissions (defaults to COM1)
    HardwareSerial(const std::wstring &comPort = L"\\\\.\\COM1");

    virtual ~HardwareSerial();

    /// \brief Indicates if the specified Serial port is ready.
    /// \return True if the specified serial port is available.
    /// \see <a href="http://arduino.cc/en/Serial/IfSerial" target="_blank">origin: Arduino::Serial::available</a>
    explicit operator bool(void);

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
    void begin(unsigned long baud, uint8_t config = SERIAL_8N1);

    /// \brief Disables serial communication, allowing the RX and TX pins to be
    /// used for general input and output.
    /// \details
    ///    To re-enable serial communication, call Serial.begin().
    /// \see <a href="http://arduino.cc/en/Serial/End" target="_blank">origin: Arduino::Serial::end</a>
    void end(void);

    /// \brief Waits for the transmission of outgoing serial data to complete.
    /// \see <a href="http://arduino.cc/en/Serial/Flush" target="_blank">origin: Arduino::Serial::flush</a>
    virtual void flush(void);

    /// \brief View the next byte
    /// \details Returns the next byte (character) of incoming serial data
    ///   without removing it from the internal serial buffer.
    /// \note
    ///   Successive calls to peek() will return the same character;
    ///   the character from the next call to read().
    /// \return The first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// \see <a href="http://arduino.cc/en/Serial/Peek" target="_blank">origin: Arduino::Serial::peek</a>
    virtual int peek(void);

    /// \brief Reads incoming serial data.
    /// \return The first byte of incoming serial data available (or -1
    ///   if no data is available)
    /// \see <a href="http://arduino.cc/en/Serial/Read" target="_blank">origin: Arduino::Serial::read</a>
    virtual int read(void);

    /// \details Sets the maximum milliseconds to wait for serial data
    /// \param [in] milliseconds Timeout duration in milliseconds
    /// \note
    ///   Defaults to 1000 milliseconds.
    /// \see Serial::parseFloat
    /// \see Serial::parseInt
    /// \see Serial::readBytes
    /// \see Serial::readBytesUntil
    /// \see <a href="http://arduino.cc/en/Serial/SetTimeout" target="_blank">origin: Arduino::Serial::setTimeout</a>
    virtual void setTimeout(unsigned long timeout);

    /// \brief Writes binary data to the serial port.
    /// \details
    ///   This data is sent as a byte or series of bytes; to send the
    ///   characters representing the digits of a number
    /// \param [in] buffer An array to send as a series of bytes
    /// \param [in] length The length of the buffer
    /// \return The number of bytes written
    /// \see HardwareSerial::print
    /// \see <a href="http://arduino.cc/en/Serial/Write" target="_blank">origin: Arduino::Serial::write</a>
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual size_t write(uint8_t c);

    inline size_t write(char c)
     {
         return write((uint8_t) c);
     }
    
     inline size_t write(const String &s)
     {
         size_t count = 0;
         for (uint16_t i = 0; i < s.length(); i++) {
             count += write(s[i]);
         }
         return count;
     }
    
     inline size_t write(const char str[])
     {
         size_t count = 0;
        for (uint16_t i = 0; i < strlen(str); i++) {
            count += write(str[i]);
        }
        return count;
     }
    
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

extern HardwareSerial Serial;    ///< This variable will provide global access to
                                 /// this pseudo-static class, and will be instantiated
                                 /// in the .cpp file.

extern HardwareSerial Serial1;   ///< This variable will provide global access to
                                 /// this pseudo-static class, and will be instantiated
                                 /// in the .cpp file.
#endif
