// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.
//
// Print.h contains functionality for print functions
// virtual size_t write(uint8_t) needs to be overwritten in any extension of Print.h
// Info on fundamental types found here: http://msdn.microsoft.com/en-us/library/cc953fe1.aspx

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Arduino.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
private:
    int write_error;
    size_t printNumber(unsigned long, uint8_t);
    size_t printFloat(double, uint8_t);

    void setWriteError(int err = 1)
    {
        write_error = err;
    }

public:
    Print() : write_error(0)
    {}

    int getWriteError()
    {
        return write_error;
    }

    void clearWriteError()
    {
        write_error = 0;
    }

    // needs to be overwritten in order to provide Print functionality
    virtual size_t write(uint8_t) = 0;

    // writes a uint8_t buffer of inputted size by calling write(uint8_t)
    virtual size_t write(const uint8_t *buffer, size_t size)
    {
        size_t count = 0;
        while (count++ < size)
        {
            if (1 != write(*buffer++))
            {
                write_error = 1;
                return 0;
            }
        }
        return count;
    }

    // calls write with a char *
    size_t write(const char *str)
    {
        if (NULL != str)
        {
            write((const uint8_t *) str, strlen(str));
        }
        else{
            return 0;
        }
    }

    // prints an array of characters
    size_t print(const char str [])
    {
        return write(str);
    }

    // prints a character (1 byte)
    size_t print(char c) { return write(c); }

    // prints an unsigned char (1 byte)
    size_t print(unsigned char c) { return write(c); }

    // prints a integer (4 bytes)
    size_t print(int n)
    {
        char buff[BUFSIZ];
        int count = sprintf_s(buff, "%d", n);
        return write((const uint8_t *) &buff, count);
    }

    // prints an unsigned int (4 bytes)
    size_t print(unsigned int n)
    {
        char buff[BUFSIZ];
        int count = sprintf_s(buff, "%d", n);
        return write((const uint8_t *) &buff, count);
    }

    // prints a long (4 bytes)
    size_t print(long n) 
    {
        char buff[BUFSIZ];
        int count = sprintf_s(buff, "%lu", n);
        return write((const uint8_t *) &buff, count);
    }

    // prints an unsigned long (4 bytes)
    size_t print(unsigned long n) 
    {
        char buff[BUFSIZ];
        int count = sprintf_s(buff, "%lu", n);
        return write((const uint8_t *) &buff, count);
    }

    // prints a double (8 bytes)
    size_t print(double n)
    {
        char buff[BUFSIZ];
        int count = sprintf_s(buff, "%g", n);
        return write((const uint8_t *) &buff, count);
    }


    // prints a character array followed by a new line
    size_t println(const char str [])
    {
        size_t count = 0;
        count += print(str);
        count += println();
        return count;
    }

    // prints a character followed by a new line
    size_t println(char c)
    {
        size_t count = 0;
        count += print(c);
        count += println();
        return count;
    }

    // prints a unsigned character followed by a new line
    size_t println(unsigned char c)
    {
        size_t count = 0;
        count += print(c);
        count += println();
        return count;
    }

    // prints an integer followed by a new line
    size_t println(int n)
    {
        size_t count = 0;
        count += print(n);
        count += println();
        return count;
    }

    // prints an unsigned integer followed by a new line
    size_t println(unsigned int n)
    {
        size_t count = 0;
        count += print(n);
        count += println();
        return count;
    }

    // prints an long followed by a new line
    size_t println(long n)
    {
        size_t count = 0;
        count += print(n);
        count += println();
        return count;
    }

    // prints an unsigned long followed by a new line
    size_t println(unsigned long n)
    {
        size_t count = 0;
        count += print(n);
        count += println();
        return count;
    }

    // prints a double followed by a new line
    size_t println(double n)
    {
        size_t count = 0;
        count += print(n);
        count += println();
        return count;
    }

    // prints a new line character
    size_t println(void) { return print('\n'); }
};