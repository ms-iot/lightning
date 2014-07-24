// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.
//
// Printable.h provides an interface class fo printing complex types

#ifndef Printable_h
#define Printable_h

#include <new.h>

class Print;

// The Printable class allows for new classes to allow implement a common printTo method
// that allows them to be printed using the print and println methods of the Print class.
class Printable
{
	public:
		virtual size_t printTo(Print &p) = 0;
};

#endif