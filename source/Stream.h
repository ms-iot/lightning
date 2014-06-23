#pragma once


class Stream
{
public:
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    size_t write(uint8_t b)
    {
        return write(&b, 1);
    }

    virtual int available() = 0;
    virtual int read() = 0;

};