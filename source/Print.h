// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.
//
// Print.h contains functionality for print functions
// virtual size_t write(uint8_t) needs to be overwritten in any extension of Print.h
// Info on fundamental types found here: http://msdn.microsoft.com/en-us/library/cc953fe1.aspx

#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include "Arduino.h"
#include "Printable.h"
#include "WString.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
private:
	int write_error;
	
	// prints out the binary representation of a number
	size_t printBinary(unsigned long n) 
	{
		char concatBuff[BUFSIZ] = {0};
		unsigned long temp = n;
		int count = 0;
		while (temp > 0)
		{
			int remainder = temp % 2;
			temp /= 2;
			char digitBuff[BUFSIZ];
			sprintf_s(digitBuff, "%d", remainder);
			strcat_s(concatBuff, digitBuff);
			count++;
		}

		for (int i = 0; i < count / 2; i++)
		{
			char temp1 = concatBuff[i];
			char temp2 = concatBuff[count - i - 1];
			concatBuff[count - 1 - i] = temp1;
			concatBuff[i] = temp2;
		}

		return write((const uint8_t *) &concatBuff, count);
	}

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
			return write((const uint8_t *) str, strlen(str));
		}
		else{
			return 0;
		}
	}

	// prints a String type
	size_t print(const String &s)
	{
		size_t count = 0;
		for (uint16_t i = 0; i < s.length(); i++) {
			count += write(s[i]);
		}
		return count;
	}

	// prints an array of characters
	size_t print(const char str[])
	{
		return write(str);
	}

	// prints a character (1 byte)
	size_t print(char c) { return write(c); }

	// prints an unsigned char (1 byte)
	size_t print(unsigned char c, int base = DEC)
	{

		return write(c);
	}

	// prints a integer (4 bytes) based on its base
	size_t print(int n, int base = DEC)
	{
		char buff[BUFSIZ];
		int count = 0;
		switch (base)
		{
		case BIN:
			return printBinary(n);
			break;
		case OCT:
			count = sprintf_s(buff, "%o", n);
			return write((const uint8_t *) &buff, count);
			break;
		case HEX:
			count = sprintf_s(buff, "%x", n);
			return write((const uint8_t *) &buff, count);
			break;
		default:
			// DEC
			count = sprintf_s(buff, "%d", n);
			return write((const uint8_t *) &buff, count);
		}
	}

	// prints an unsigned int (4 bytes) based on its base
	size_t print(unsigned int n, int base = DEC)
	{
		char buff[BUFSIZ];
		int count = 0;
		switch (base)
		{
		case BIN:
			return printBinary(n);
			break;
		case OCT:
			count = sprintf_s(buff, "%o", n);
			return write((const uint8_t *) &buff, count);
			break;
		case HEX:
			count = sprintf_s(buff, "%x", n);
			return write((const uint8_t *) &buff, count);
			break;
		default:
			// DEC
			count = sprintf_s(buff, "%d", n);
			return write((const uint8_t *) &buff, count);
			break;
		}
	}

	// prints a long (4 bytes) based on its base
	size_t print(long n, int base = DEC)
	{
		char buff[BUFSIZ];
		int count = 0;
		switch (base)
		{
		case BIN:
			return printBinary(n);
			break;
		case OCT:
			count = sprintf_s(buff, "%lo", n);
			return write((const uint8_t *) &buff, count);
			break;
		case HEX:
			count = sprintf_s(buff, "%lx", n);
			return write((const uint8_t *) &buff, count);
			break;
		default:
			// DEC
			count = sprintf_s(buff, "%ld", n);
			return write((const uint8_t *) &buff, count);
			break;
		}
	}

	// prints an unsigned long (4 bytes) based on its base
	size_t print(unsigned long n, int base = DEC)
	{
		char buff[BUFSIZ];
		int count = 0;
		switch (base)
		{
		case BIN:
			return printBinary(n);
			break;
		case OCT:
			count = sprintf_s(buff, "%lo", n);
			return write((const uint8_t *) &buff, count);
			break;
		case HEX:
			count = sprintf_s(buff, "%lx", n);
			return write((const uint8_t *) &buff, count);
			break;
		default:
			// DEC
			count = sprintf_s(buff, "%ld", n);
			return write((const uint8_t *) &buff, count);
			break;
		}
	}

	// prints a double (8 bytes) with trailing digits
	size_t print(double n, int digits = 2)
	{
		int count = 0;

		// handles negative numbers
		if (n < 0)
		{
			count += print('-');
			n = -n;
		}

		// handles the integer part
		unsigned long intPart = (unsigned long) n;
		double remainder = n - intPart;
		count += print(intPart);

		// handles the decimal point
		if (digits > 0) count += print('.');

		// handles the decimal part
		for (int i = 0; i < digits; i++)
		{
			remainder *= 10;
			intPart = (unsigned long) remainder;
			count += print(intPart);
			remainder -= intPart;
		}

		return count;
	}

	// prints a printable object
	size_t print(Printable& o)
	{
		return o.printTo(*this);
	}

	// prints a String type followed by a new line
	size_t println(const String &s)
	{
		size_t count = 0;
		count += print(s);
		count += println();
		return count;
	}

	// prints a character array followed by a new line
	size_t println(const char str[])
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
	size_t println(unsigned char c, int base = DEC)
	{
		size_t count = 0;
		count += print(c, base);
		count += println();
		return count;
	}

	// prints an integer followed by a new line
	size_t println(int n, int base = DEC)
	{
		size_t count = 0;
		count += print(n, base);
		count += println();
		return count;
	}

	// prints an unsigned integer followed by a new line
	size_t println(unsigned int n, int base = DEC)
	{
		size_t count = 0;
		count += print(n, base);
		count += println();
		return count;
	}

	// prints an long followed by a new line
	size_t println(long n, int base = DEC)
	{
		size_t count = 0;
		count += print(n, base);
		count += println();
		return count;
	}

	// prints an unsigned long followed by a new line
	size_t println(unsigned long n, int base = DEC)
	{
		size_t count = 0;
		count += print(n, base);
		count += println();
		return count;
	}

	// prints a double followed by a new line
	size_t println(double n, int digits = 2)
	{
		size_t count = 0;
		count += print(n, digits);
		count += println();
		return count;
	}

	// prints a Printable object followed by a new line
	size_t println(Printable& o)
	{
		size_t count = 0;
		count += print(o);
		count += println();
		return 0;
	}

	// prints a new line character
	size_t println(void) { return print('\n'); }
};