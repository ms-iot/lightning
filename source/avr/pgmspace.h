// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef PGM_SPACE_H
#define PGM_SPACE_H

#include <cstdint>

/*
 * The avr/pgmspace.h file providies a set of macros
 * necessary for accessing program memory space in the
 * Arduino (in AVR chips program memory is located in
 * a different physical location than the data memory).
 * Here, we have a flat memory space and we are
 * stripping out these macros to allow for an easy
 * conversion of Arduino specific code.
 */

#define pgm_read_byte_near(address_short) *reinterpret_cast<uint8_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_word_near(address_short) *reinterpret_cast<uint16_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_dword_near(address_short) *reinterpret_cast<uint32_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_float_near(address_short) *reinterpret_cast<float *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))

#define pgm_read_byte_far(address_long) *reinterpret_cast<uint8_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint32_t>(address_long))
#define pgm_read_word_far(address_long) *reinterpret_cast<uint16_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint32_t>(address_long))
#define pgm_read_dword_far(address_long) *reinterpret_cast<uint32_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint32_t>(address_long))
#define pgm_read_float_far(address_long) *reinterpret_cast<float *>(reinterpret_cast<void *>(NULL) | static_cast<uint32_t>(address_long))

#define pgm_read_byte(address_short) *reinterpret_cast<uint8_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_word(address_short) *reinterpret_cast<uint16_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_dword(address_short) *reinterpret_cast<uint32_t *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))
#define pgm_read_float(address_short) *reinterpret_cast<float *>(reinterpret_cast<void *>(NULL) | static_cast<uint16_t>(address_short))

#endif
