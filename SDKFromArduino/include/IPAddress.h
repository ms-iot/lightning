/*
  IPAddress.h - Base class that provides IPAddress
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

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

#ifndef IPAddress_h
#define IPAddress_h

#include <stdint.h>
#include "Printable.h"
#include "WString.h"
#include "Lightning.h"

// A class to make it easier to handle and pass around IP addresses

class IPAddress : public Printable {
private:
    union {
        uint8_t bytes[4];  // IPv4 address
        uint32_t dword;
    } _address;

    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
    uint8_t* raw_address() { return _address.bytes; };

public:
    // Constructors
    LIGHTNING_DLL_API IPAddress();
    LIGHTNING_DLL_API IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    LIGHTNING_DLL_API IPAddress(uint32_t address);
    LIGHTNING_DLL_API IPAddress(const uint8_t *address);

    LIGHTNING_DLL_API bool fromString(const char *address);
    LIGHTNING_DLL_API bool fromString(const String &address) { return fromString(address.c_str()); }

    // Overloaded cast operator to allow IPAddress objects to be used where a pointer
    // to a four-byte uint8_t array is expected
    LIGHTNING_DLL_API operator uint32_t() const { return _address.dword; };
    LIGHTNING_DLL_API bool operator==(const IPAddress& addr) const { return _address.dword == addr._address.dword; };
    LIGHTNING_DLL_API bool operator==(const uint8_t* addr) const;

    // Overloaded index operator to allow getting and setting individual octets of the address
    LIGHTNING_DLL_API uint8_t operator[](int index) const { return _address.bytes[index]; };
    LIGHTNING_DLL_API uint8_t& operator[](int index) { return _address.bytes[index]; };

    // Overloaded copy operators to allow initialisation of IPAddress objects from other types
    LIGHTNING_DLL_API IPAddress& operator=(const uint8_t *address);
    LIGHTNING_DLL_API IPAddress& operator=(uint32_t address);

    LIGHTNING_DLL_API virtual size_t printTo(Print& p) const;

    friend class EthernetClass;
    friend class UDP;
    friend class Client;
    friend class Server;
    friend class DhcpClass;
    friend class DNSClient;
};

const IPAddress INADDR_NONE(0,0,0,0);

#endif
