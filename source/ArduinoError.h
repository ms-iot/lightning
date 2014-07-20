// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef ARDUINO_ERROR_H
#define ARDUINO_ERROR_H

#include <stdexcept>
#include <strsafe.h>

class _arduino_fatal_error : public std::runtime_error
{
public:
	typedef std::runtime_error _Mybase;

	explicit _arduino_fatal_error(const char *_Message)
		: _Mybase(_Message)
	{    // construct from message string
	}
};

class _arduino_quit_exception : public std::exception { };

inline void _exit_arduino_loop()
{
	throw _arduino_quit_exception();
}

inline void ThrowError(_In_ _Printf_format_string_ STRSAFE_LPCSTR pszFormat, ...)
{
	HRESULT hr;
	char buf[BUFSIZ];

	va_list argList;

	va_start(argList, pszFormat);

	hr = StringCbVPrintfA(buf,
		ARRAYSIZE(buf),
		pszFormat,
		argList);

	va_end(argList);

	if (SUCCEEDED(hr))
	{
		throw _arduino_fatal_error(buf);
	}
	else
	{
		throw _arduino_fatal_error("StringCbVPrintfA() failed while attempting to print exception message");
	}
}

#endif
