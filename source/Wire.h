/** \file wire.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.
 * See License.txt in the project root for license information.
 */

#ifndef TWO_WIRE_H
#define TWO_WIRE_H

#include <Windows.h>
#include <stdint.h>
#include <vector>

#include "ArduinoError.h"
#include "embprpusr.h"
#include "galileo.h"

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

#define BUFFER_LENGTH 32

// Forward declaration(s):
int Log(const char *format, ...);

class TwoWire
{

public:
    enum TwiError {
        SUCCESS = 0,
        TWI_BUFFER_OVERRUN = 1,
        ADDR_NACK_RECV = 2,
        DATA_NACK_RECV = 3,
        OTHER_ERROR = 4,
    };

    TwoWire() :
        i2c(nullptr),
        i2cHasBeenEnabled(false),
        connectionSlaveAddress(0),
        slaveWriteAddress(0),
        writeBuf(),
        readBuf(),
        readIndex()
    {
        this->readIndex = readBuf.end();
    }

    virtual ~TwoWire()
    {
        EnableI2C(false);

        if (this->i2c != nullptr)
        {
            I2CFree(i2c);
            this->i2c = nullptr;
        }
    }

    /// \brief Initiate the Wire library and join the I2C bus as a master or slave.
    /// \note The Galileo cannot operate in slave mode
    /// \see <a href="http://arduino.cc/en/Reference/WireBegin" target="_blank">origin: Arduino::Wire::begin</a>
    void begin()
    {
        if (this->i2c != nullptr)
        {
            I2CFree(i2c);
            this->i2c = nullptr;
        }

        this->connectionSlaveAddress = 0;
        this->slaveWriteAddress = 0;
        this->writeBuf.reserve(BUFFER_LENGTH);
        this->readBuf.reserve(BUFFER_LENGTH);
        this->readIndex = readBuf.end();

        EnableI2C(true);
    }

    /// \brief Begin a transmission to the I2C slave device with the given address.
    /// \param [in] slaveAddress The 7-bit device address where the data should be sent
    /// \note  Subsequently, queue bytes for transmission with the write() function
    /// and transmit them by calling endTransmission().
    /// \see Wire::write
    /// \see Wire::endTransmission
    /// \see <a href="http://arduino.cc/en/Reference/WireBeginTransmission" target="_blank">origin: Arduino::Wire::beginTransmission</a>
    void beginTransmission(int slaveAddress)
    {
        _UpdateConnection(static_cast<ULONG>(slaveAddress));

        this->slaveWriteAddress = slaveAddress;
        this->writeBuf.clear();
    }

    /// \brief Ends a transmission to a slave device
    /// \details Ends a transmission started by beginTransmission() and transmits
    /// the bytes that were queued by write().
    /// \param [in] [sendStop] TRUE or FALSE (default is true)
    /// \n If true, endTransmission() sends a stop message after transmission, releasing
    /// the I2C bus. If false, endTransmission() sends a restart message after transmission.
    /// The bus will not be released, which prevents another master device from transmitting
    /// between messages.This allows one master device to send multiple transmissions while
    /// in control.
    /// \return The status of the transmission
    /// \arg 0 - success
    /// \arg 1 - data too long to fit in transmit buffer
    /// \arg 2 - received NACK on transmit of address
    /// \arg 3 - received NACK on transmit of data
    /// \arg 4 - other error
    /// \see Wire::beginTransmission
    /// \see Wire::write
    /// \see <a href="http://arduino.cc/en/Reference/WireEndTransmission" target="_blank">origin: Arduino::Wire::endTransmission</a>
    int endTransmission(int sendStop)
    {
        DWORD bytesWritten = 0;

        if (sendStop)
        {
            HRESULT hr = I2CWrite(
                this->i2c,
                this->writeBuf.data(),
                this->writeBuf.size(),
                &bytesWritten);

            if (FAILED(hr))
            {
                return ADDR_NACK_RECV;
            }

            this->slaveWriteAddress = 0;
        }

        return SUCCESS;
    }

    int endTransmission(void)
    {
        return this->endTransmission(1);
    }

    /// \brief Used to request bytes from a slave device
    /// \note The bytes may then be retrieved with the available()
    /// and read() functions.
    /// \param [in] address The 7-bit address of the device to request bytes from 
    /// \param [in] quantity The number of bytes to request
    /// \param [in] sendStop True will send a stop message after the request,
    /// releasing the bus. false will continually send a restart after the request,
    /// keeping the connection active.
    /// \return The number of bytes returned from the slave device
    /// \see Wire::available
    /// \see Wire::read
    /// \see <a href="http://arduino.cc/en/Reference/WireRequestFrom" target="_blank">origin: Arduino::Wire::requestFrom</a>
    int requestFrom(int address, int quantity, int sendStop)
    {
        DWORD bytesReturned;

        UNREFERENCED_PARAMETER(sendStop);

        if (quantity < 0)
            ThrowError("quantity must be positive: %d", quantity);

        _UpdateConnection(address);

        this->readBuf.resize(quantity);

        // is there a writeread sequence pending?
        if (this->slaveWriteAddress == address)
        {
            // if user previously called endTransmission with sendStop=0,
            // they are most likely doing a write/read sequence.
            // Until generic sequence transfers are implemented using the 
            // SPB controller lock mechanism, we can support the special
            // case of a single write/read sequence which covers the
            // majority of uses that require repeated starts.
            HRESULT hr = I2CWriteReadAtomic(
                this->i2c,
                this->writeBuf.data(),
                this->writeBuf.size(),
                this->readBuf.data(),
                this->readBuf.size(),
                &bytesReturned);

            if (FAILED(hr))
            {
                ThrowError("I2C_CONTROLLER IO failed");
            }

            // indicate that pending write has been flushed
            this->slaveWriteAddress = 0;
        }
        else
        {
            HRESULT hr = I2CRead(
                this->i2c,
                this->readBuf.data(),
                this->readBuf.size(),
                &bytesReturned);

            if (FAILED(hr))
            {
                ThrowError("I2C_CONTROLLER IO failed");
            }
        }

        this->readBuf.resize(bytesReturned);
        this->readIndex = this->readBuf.begin();

        return bytesReturned;
    }

    int requestFrom(int address, int quantity)
    {
        return this->requestFrom(address, quantity, 1);
    }

    void setI2cHasBeenEnabled(bool enable)
    {
        i2cHasBeenEnabled = enable;
    }

    bool getI2cHasBeenEnabled()
    {
        return i2cHasBeenEnabled;
    }

    /// \brief Registers a function to be called when a slave device receives a transmission
    /// \param [in] handler The function to be called when the slave receives data;
    /// should take a single int parameter (the number of bytes read from the master)
    /// and return nothing
    /// \see Wire::onRequest
    /// \see <a href="http://arduino.cc/en/Reference/WireOnReceive" target="_blank">origin: Arduino::Wire::onReceive</a>
    void onReceive(void(*handler)(int))
    {
        Log("FEATURE UNAVAILABLE: Galileo cannot act as I2C slave device!");
    }
    
    /// \brief Register a function to be called when a master requests data
    /// \param [in] handler The function to be called, takes no parameters and returns nothing
    /// \see Wire::onReceive
    /// \see <a href="http://arduino.cc/en/Reference/WireOnRequest" target="_blank">origin: Arduino::Wire::onRequest</a>
    void onRequest(void(*handler)(void))
    {
        Log("FEATURE UNAVAILABLE: Galileo cannot act as I2C slave device!");
    }

    /// \brief Returns the number of bytes available for retrieval with read()
    /// \details This should be called on a master device after a call to
    /// requestFrom() or on a slave inside the onReceive() handler.
    /// \return The number of bytes available for reading.
    /// \see Wire::read
    /// \see Stream::available
    /// \see <a href="http://arduino.cc/en/Reference/WireAvailable" target="_blank">origin: Arduino::Wire::available</a>
    virtual int available(void)
    {
        return this->readBuf.end() - this->readIndex;
    }

    /// \brief Reads a byte that was transmitted on the I2C bus
    /// \return The next byte received
    /// \see Wire::write
    /// \see Wire::available
    /// \see Wire::requestFrom
    /// \see Stream::read
    /// \see <a href="http://arduino.cc/en/Reference/WireRead" target="_blank">origin: Arduino::Wire::read</a>
    virtual int read(void)
    {
        int data = -1;

        if (this->readIndex != this->readBuf.end())
        {
            data = *this->readIndex++;
        }

        return data;
    }

    virtual int peek(void)
    {
        int data = -1;

        if (this->readIndex != this->readBuf.end())
        {
            data = *this->readIndex;
        }

        return data;
    }

    virtual void flush(void)
    {

    }

    /// \brief Writes data on the I2C bus
    /// \param [in] data An array of data to send as bytes
    /// \param [in] cbData The number of bytes to transmit
    /// \return The number of bytes written
    /// \see <a href="http://arduino.cc/en/Reference/WireWrite" target="_blank">origin: Arduino::Wire::write</a>
    virtual size_t write(const uint8_t *data, size_t cbData)
    {
        buffer_t::iterator end = this->writeBuf.end();
        this->writeBuf.resize(this->writeBuf.size() + cbData);
        std::copy_n(data, cbData, end);

        return cbData;
    }

    virtual size_t write(uint8_t data)
    {
        this->writeBuf.push_back(data);
        return 1;
    }

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }

    I2C_CONTROLLER *handle() const
    {
        return this->i2c;
    }

private:

    typedef std::vector<uint8_t> buffer_t;

    // Ensures that the I2C connection handle is opened for
    // the specified slave address
    void _UpdateConnection(ULONG address)
    {
        const ULONG I2C_CONNECTION_SPEED = TWI_FREQ;

        // if the address is different, we need to reopen the connection
        if (this->connectionSlaveAddress != address)
        {
            // if a connection is open, close it
            if (this->i2c != nullptr)
            {
                I2CFree(this->i2c);
                this->i2c = nullptr;
            }

            HRESULT hr = I2CCreateInstance(
                I2C_CONTROLLER_INDEX,
                address,
                I2C_CONNECTION_SPEED,
                &this->i2c);

            if (FAILED(hr))
            {
                ThrowError("Failed to create I2C_CONTROLLER instance");
            }

            this->connectionSlaveAddress = address;
        }
    }

    void EnableI2C(bool enable)
    {
        HRESULT hr = GpioSetDir(GPORT1_BIT5, 1);
        if (FAILED(hr))
        {
            ThrowError("Failed to configure I2C_CONTROLLER mux");
        }

        hr = GpioWrite(GPORT1_BIT5, enable ? 0 : 1);
        if (FAILED(hr))
        {
            ThrowError("Failed to configure I2C_CONTROLLER mux");
        }
        i2cHasBeenEnabled = enable;
    }

    I2C_CONTROLLER *i2c;

    // stores the fact that I2C has been explicitely enabled.
    bool i2cHasBeenEnabled;

    // stores the slave address that is currently opened by I2C handle
    ULONG connectionSlaveAddress;

    // stores the slave address to write to. This is set
    // in beginTransmission() and gets passed to the 
    // driver in endTransmission() when the data is finally flushed
    int slaveWriteAddress;

    // buffer for containing data to send to a slave device
    // calling beginTransmission() initiates "buffer fill mode",
    // where subsequent calls to write() will append data to 
    // the buffer. No data is written to the device until
    // endTransmission() is called.
    buffer_t writeBuf;

    // buffer for containing data read from a slave device
    buffer_t readBuf;
    buffer_t::const_iterator readIndex;
};

__declspec(selectany) TwoWire Wire;

#endif
