// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

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

    // slave mode not supported
    // void begin(uint8_t);
    // void begin(int);

    void beginTransmission(int slaveAddress)
    {
        _UpdateConnection(static_cast<ULONG>(slaveAddress));

        this->slaveWriteAddress = slaveAddress;
        this->writeBuf.clear();
    }

    int endTransmission(void)
    {
        return this->endTransmission(1);
    }

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

    int requestFrom(int address, int quantity)
    {
        return this->requestFrom(address, quantity, 1);
    }

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

    bool getI2cHasBeenEnabled()
    {
        return i2cHasBeenEnabled;
    }

    void setI2cHasBeenEnabled(bool enable)
    {
        i2cHasBeenEnabled = enable;
    }

    virtual size_t write(uint8_t data)
    {
        this->writeBuf.push_back(data);
        return 1;
    }

    virtual size_t write(const uint8_t *data, size_t cbData)
    {
        buffer_t::iterator end = this->writeBuf.end();
        this->writeBuf.resize(this->writeBuf.size() + cbData);
        std::copy_n(data, cbData, end);

        return cbData;
    }

    virtual int available(void)
    {
        return this->readBuf.end() - this->readIndex;
    }

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
