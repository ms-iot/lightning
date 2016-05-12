/*
  WString.h - String library for Wiring & Arduino
  ...mostly rewritten by Paul Stoffregen...
  Copyright (c) 2009-10 Hernando Barragan.  All right reserved.
  Copyright 2011, Paul Stoffregen, paul@pjrc.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef String_class_h
#define String_class_h
#ifdef __cplusplus

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include "Lightning.h"

// When compiling programs with this class, the following gcc parameters
// dramatically increase performance and memory (RAM) efficiency, typically
// with little or no increase in code size.
//     -felide-constructors
//     -std=c++0x

class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;

// The string class
class String
{
	// use a function pointer to allow for "if (s)" without the
	// complications of an operator bool(). for more information, see:
	// http://www.artima.com/cppsource/safebool.html
	typedef void (String::*StringIfHelperType)() const;
	void StringIfHelper() const {}

public:
	// constructors
	// creates a copy of the initial value.
	// if the initial value is null or invalid, or if memory allocation
	// fails, the string will be marked as invalid (i.e. "if (s)" will
	// be false).
	LIGHTNING_DLL_API String(const char *cstr = "");
	LIGHTNING_DLL_API String(const String &str);
	LIGHTNING_DLL_API String(const __FlashStringHelper *str);
	#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
	LIGHTNING_DLL_API String(String &&rval);
	LIGHTNING_DLL_API String(StringSumHelper &&rval);
	#endif
	LIGHTNING_DLL_API explicit String(char c);
	LIGHTNING_DLL_API explicit String(unsigned char, unsigned char base=10);
	LIGHTNING_DLL_API explicit String(int, unsigned char base=10);
	LIGHTNING_DLL_API explicit String(unsigned int, unsigned char base=10);
	LIGHTNING_DLL_API explicit String(long, unsigned char base=10);
	LIGHTNING_DLL_API explicit String(unsigned long, unsigned char base=10);
	LIGHTNING_DLL_API explicit String(float, unsigned char decimalPlaces=2);
	LIGHTNING_DLL_API explicit String(double, unsigned char decimalPlaces=2);
	LIGHTNING_DLL_API ~String(void);

	// memory management
	// return true on success, false on failure (in which case, the string
	// is left unchanged).  reserve(0), if successful, will validate an
	// invalid string (i.e., "if (s)" will be true afterwards)
	LIGHTNING_DLL_API unsigned char reserve(unsigned int size);
	inline unsigned int length(void) const {return len;}

	// creates a copy of the assigned value.  if the value is null or
	// invalid, or if the memory allocation fails, the string will be 
	// marked as invalid ("if (s)" will be false).
	LIGHTNING_DLL_API String & operator = (const String &rhs);
	LIGHTNING_DLL_API String & operator = (const char *cstr);
	LIGHTNING_DLL_API String & operator = (const __FlashStringHelper *str);
	#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
	LIGHTNING_DLL_API String & operator = (String &&rval);
	LIGHTNING_DLL_API String & operator = (StringSumHelper &&rval);
	#endif

	// concatenate (works w/ built-in types)
	
	// returns true on success, false on failure (in which case, the string
	// is left unchanged).  if the argument is null or invalid, the 
	// concatenation is considered unsucessful.  
	LIGHTNING_DLL_API unsigned char concat(const String &str);
	LIGHTNING_DLL_API unsigned char concat(const char *cstr);
	LIGHTNING_DLL_API unsigned char concat(char c);
	LIGHTNING_DLL_API unsigned char concat(unsigned char c);
	LIGHTNING_DLL_API unsigned char concat(int num);
	LIGHTNING_DLL_API unsigned char concat(unsigned int num);
	LIGHTNING_DLL_API unsigned char concat(long num);
	LIGHTNING_DLL_API unsigned char concat(unsigned long num);
	LIGHTNING_DLL_API unsigned char concat(float num);
	LIGHTNING_DLL_API unsigned char concat(double num);
	LIGHTNING_DLL_API unsigned char concat(const __FlashStringHelper * str);
	
	// if there's not enough memory for the concatenated value, the string
	// will be left unchanged (but this isn't signalled in any way)
	String & operator += (const String &rhs)	{concat(rhs); return (*this);}
	String & operator += (const char *cstr)		{concat(cstr); return (*this);}
	String & operator += (char c)			{concat(c); return (*this);}
	String & operator += (unsigned char num)		{concat(num); return (*this);}
	String & operator += (int num)			{concat(num); return (*this);}
	String & operator += (unsigned int num)		{concat(num); return (*this);}
	String & operator += (long num)			{concat(num); return (*this);}
	String & operator += (unsigned long num)	{concat(num); return (*this);}
	String & operator += (float num)		{concat(num); return (*this);}
	String & operator += (double num)		{concat(num); return (*this);}
	String & operator += (const __FlashStringHelper *str){concat(str); return (*this);}

	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, const String &rhs);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, const char *cstr);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, char c);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned char num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, int num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned int num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, long num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, float num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, double num);
	LIGHTNING_DLL_API friend StringSumHelper & operator + (const StringSumHelper &lhs, const __FlashStringHelper *rhs);

	// comparison (only works w/ Strings and "strings")
	operator StringIfHelperType() const { return buffer ? &String::StringIfHelper : 0; }
	LIGHTNING_DLL_API int compareTo(const String &s) const;
	LIGHTNING_DLL_API unsigned char equals(const String &s) const;
	LIGHTNING_DLL_API unsigned char equals(const char *cstr) const;
	unsigned char operator == (const String &rhs) const {return equals(rhs);}
	unsigned char operator == (const char *cstr) const {return equals(cstr);}
	unsigned char operator != (const String &rhs) const {return !equals(rhs);}
	unsigned char operator != (const char *cstr) const {return !equals(cstr);}
	LIGHTNING_DLL_API unsigned char operator <  (const String &rhs) const;
	LIGHTNING_DLL_API unsigned char operator >  (const String &rhs) const;
	LIGHTNING_DLL_API unsigned char operator <= (const String &rhs) const;
	LIGHTNING_DLL_API unsigned char operator >= (const String &rhs) const;
	LIGHTNING_DLL_API unsigned char equalsIgnoreCase(const String &s) const;
	LIGHTNING_DLL_API unsigned char startsWith( const String &prefix) const;
	LIGHTNING_DLL_API unsigned char startsWith(const String &prefix, unsigned int offset) const;
	LIGHTNING_DLL_API unsigned char endsWith(const String &suffix) const;

	// character acccess
	LIGHTNING_DLL_API char charAt(unsigned int index) const;
	LIGHTNING_DLL_API void setCharAt(unsigned int index, char c);
	LIGHTNING_DLL_API char operator [] (unsigned int index) const;
	LIGHTNING_DLL_API char& operator [] (unsigned int index);
	LIGHTNING_DLL_API void getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index=0) const;
	void toCharArray(char *buf, unsigned int bufsize, unsigned int index=0) const
		{getBytes((unsigned char *)buf, bufsize, index);}
	const char * c_str() const { return buffer; }

	// search
	LIGHTNING_DLL_API int indexOf( char ch ) const;
	LIGHTNING_DLL_API int indexOf( char ch, unsigned int fromIndex ) const;
	LIGHTNING_DLL_API int indexOf( const String &str ) const;
	LIGHTNING_DLL_API int indexOf( const String &str, unsigned int fromIndex ) const;
	LIGHTNING_DLL_API int lastIndexOf( char ch ) const;
	LIGHTNING_DLL_API int lastIndexOf( char ch, unsigned int fromIndex ) const;
	LIGHTNING_DLL_API int lastIndexOf( const String &str ) const;
	LIGHTNING_DLL_API int lastIndexOf( const String &str, unsigned int fromIndex ) const;
	String substring( unsigned int beginIndex ) const { return substring(beginIndex, len); };
	LIGHTNING_DLL_API String substring( unsigned int beginIndex, unsigned int endIndex ) const;

	// modification
	LIGHTNING_DLL_API void replace(char find, char replace);
	LIGHTNING_DLL_API void replace(const String& find, const String& replace);
	LIGHTNING_DLL_API void remove(unsigned int index);
	LIGHTNING_DLL_API void remove(unsigned int index, unsigned int count);
	LIGHTNING_DLL_API void toLowerCase(void);
	LIGHTNING_DLL_API void toUpperCase(void);
	LIGHTNING_DLL_API void trim(void);

	// parsing/conversion
	LIGHTNING_DLL_API long toInt(void) const;
	LIGHTNING_DLL_API float toFloat(void) const;

protected:
	char *buffer;	        // the actual char array
	unsigned int capacity;  // the array length minus one (for the '\0')
	unsigned int len;       // the String length (not counting the '\0')
protected:
	void init(void);
	void invalidate(void);
	unsigned char changeBuffer(unsigned int maxStrLen);
	unsigned char concat(const char *cstr, unsigned int length);

	// copy and move
	String & copy(const char *cstr, unsigned int length);
	String & copy(const __FlashStringHelper *pstr, unsigned int length);
       #if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
	void move(String &rhs);
	#endif
};

class StringSumHelper : public String
{
public:
	StringSumHelper(const String &s) : String(s) {}
	StringSumHelper(const char *p) : String(p) {}
	StringSumHelper(char c) : String(c) {}
	StringSumHelper(unsigned char num) : String(num) {}
	StringSumHelper(int num) : String(num) {}
	StringSumHelper(unsigned int num) : String(num) {}
	StringSumHelper(long num) : String(num) {}
	StringSumHelper(unsigned long num) : String(num) {}
	StringSumHelper(float num) : String(num) {}
	StringSumHelper(double num) : String(num) {}
};

#endif  // __cplusplus
#endif  // String_class_h
