// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef PGM_SPACE_H
#define PGM_SPACE_H

#include <cstdint>
#include <cstring>

/*
 * The avr/pgmspace.h file providies a set of macros
 * necessary for accessing program memory space in the
 * Arduino (in AVR chips program memory is located in
 * a different physical location than the data memory).
 * Here, we have a flat memory space and we are
 * stripping out these macros to allow for an easy
 * conversion of Arduino specific code.
 */

// Not defined in Windows
#define PROGMEM

typedef const char* PGM_P;
typedef const void* PGM_VOID_P;
typedef void prog_void;
typedef char prog_char;
typedef unsigned char prog_uchar;
typedef int8_t prog_int8_t;
typedef uint8_t prog_uint8_t;
typedef int16_t prog_int16_t;
typedef uint16_t prog_uint16_t;
typedef int32_t rog_int32_t;
typedef uint32_t prog_uint32_t;
typedef int64_t prog_int64_t;
typedef uint64_t prog_uint64_t;
typedef char* uint_farptr_t;

inline uint8_t pgm_read_byte(PGM_P address)
{
	return *reinterpret_cast<uint8_t *>(const_cast<char*>(address));
}

inline uint16_t pgm_read_word(PGM_P address)
{
	return *reinterpret_cast<uint16_t *>(const_cast<char*>(address));
}

inline uint32_t pgm_read_dword(PGM_P address)
{
	return *reinterpret_cast<uint32_t *>(const_cast<char*>(address));
} 

inline float pgm_read_float(PGM_P address)
{
	return *reinterpret_cast<float *>(const_cast<char*>(address));
}

inline void* pgm_read_ptr(PGM_P address)
{
	return reinterpret_cast<void *>(const_cast<char*>(address));
}

inline uint8_t pgm_read_byte(PGM_VOID_P address)
{
	return *const_cast<uint8_t*> (reinterpret_cast<const uint8_t *>(address));
}

inline uint16_t pgm_read_word(PGM_VOID_P address)
{
	return *const_cast<uint16_t*> (reinterpret_cast<const uint16_t *>(address));
}

inline uint32_t pgm_read_dword(PGM_VOID_P address)
{
	return *const_cast<uint32_t*> (reinterpret_cast<const uint32_t *>(address));
} 

inline float pgm_read_float(PGM_VOID_P address)
{
	return *const_cast<float*> (reinterpret_cast<const float *>(address));
}

inline void* pgm_read_ptr(PGM_VOID_P address)
{
	return const_cast<void*>(reinterpret_cast<const void*>(address));
}

#define PSTR(s)  ((const char *)(s)
#define pgm_read_byte_near(address)  pgm_read_byte(address)
#define pgm_read_word_near(address)  pgm_read_word(address)
#define pgm_read_dword_near(address) pgm_read_dword(address)
#define pgm_read_float_near(address) pgm_read_float(address)
#define pgm_read_ptr_near(address)   pgm_ptr_float(address)

#define pgm_read_byte_far(address)  pgm_read_byte(address)
#define pgm_read_word_far(address)  pgm_read_word(address)
#define pgm_read_dword_far(address) pgm_read_dword(address)
#define pgm_read_float_far(address) pgm_read_float(address)
#define pgm_read_ptr_far(address)   pgm_ptr_float(address)

#define pgm_get_far_address(var)    (&(var))

#pragma warning( push )

// Needed for CRT deprecated functions
#pragma warning(disable : 4996)

// PGM versions of string functions can use regular string functions on Windows

inline const void * memchr_P(const void* buf, int ch, size_t len)
{
    return memchr(buf, ch, len);
}

inline int memcmp_P(const void * buf1, const void * buf2, size_t len)
{
    return memcmp(buf1, buf2, len);
}

inline void * memccpy_P(void * dest, const void * src, int val, size_t len)
{
    return memccpy(dest, src, val, len);
}

inline void * memcpy_P(void * dest, const void * src, size_t len)
{
    return memcpy(dest, src, len);
}

inline void * memmem_P(const void * buf1, size_t buf1_len, const void * buf2, size_t buf2_len)
{
    const unsigned char * p = reinterpret_cast<const unsigned char *>(buf1);
    size_t plen = buf1_len;
    int ch;

    if (!buf2_len)
        return nullptr;

    ch = *reinterpret_cast<const unsigned char *>(buf2);

    while (plen >= buf2_len && (p = reinterpret_cast<const unsigned char *>(memchr(p, ch, plen - buf2_len + 1))))
    {
        if (!memcmp(p, buf2, buf2_len))
        {
            return reinterpret_cast<void *>(const_cast<unsigned char *>(p));
        }

        p++;
        plen = buf1_len - (p - reinterpret_cast<const unsigned char *>(buf1));
    }

    return nullptr;
}

inline size_t strlen_P(const char *str)
{
    return strlen(str);
}

inline char * strcat_P(char * dest, const char * src)
{
    return strcat(dest, src);
}

inline const char * strchr_P(const char * str, int ch)
{
    return strchr(str, ch);
}

inline const char * strchrnul_P(const char * str, int c)
{
    const char* ret = strchr(str, c);
    return ret ? ret : (str + strlen(str));
}

inline int strcmp_P(const char * str1, const char * str2)
{
    return strcmp(str1, str2);
}

inline char* strcpy_P(char* dest, const char * src)
{
    return strcpy(dest, src);
}

inline int strcasecmp_P(const char * s1, const char * s2)
{
    return stricmp(s1, s2);
}

inline const char * strcasestr_P(const char * s1, const char * s2)
{
    if (s1 == nullptr || s2 == nullptr)
        return nullptr;

    const char* p1 = s1;
    const char* p2 = s2;
    const char* r = *p2 == '\0' ? s1 : nullptr;

    while (*p1 != '\0' && *p2 != '\0')
    {
        if (tolower(*p1) == tolower(*p2))
        {
            if (r == nullptr)
            {
                r = p1;
            }

            p2++;
        }
        else
        {
            p2 = s2;
            if (tolower(*p1) == tolower(*p2))
            {
                r = p1;
                p2++;
            }
            else
            {
                r = nullptr;
            }
        }

        p1++;
    }

    return (*p2 == '\0' ? r : nullptr);
}

inline size_t strcspn_P(const char * str, const char *reject)
{
    return strcspn(str, reject);
}

inline size_t strnlen_P(const char * str, size_t len)
{
    return strnlen(str, len);
}

inline int strncmp_P(const char * s1, const char * s2, size_t len)
{
    return strncmp(s1, s2, len);
}

inline int strncasecmp_P(const char * s1, const char * s2, size_t len)
{
    return strnicmp(s1, s2, len);
}

inline char * strncat_P(char * s1, const char * s2, size_t len)
{
    return strncat(s1, s2, len);
}

inline char * strncpy_P(char * dest, const char * src, size_t len)
{
    return strncpy(dest, src, len);
}

inline const char * strpbrk_P(const char *str, const char *accept)
{
    return strpbrk(str, accept);
}

inline size_t strspn_P(const char * str, const char * accept)
{
    return strspn(str, accept);
}

inline const char * strstr_P(const char * s1, const char * s2)
{
    return strstr(s1, s2);
}

inline char * strtok_P(char * str, const char * delim)
{
    return strtok(str, delim);
}

inline size_t strlen_PF(uint_farptr_t str)
{
    return strlen(str);
}

inline size_t strnlen_PF(uint_farptr_t str, size_t len)
{
    return strnlen(str, len);
}

inline void * memcpy_PF(void *dest, uint_farptr_t src, size_t len)
{
    return memcpy(dest, src, len);
}

inline char * strcpy_PF(char *dest, uint_farptr_t src)
{
    return strcpy(dest, src);
}

inline char * strncpy_PF(char *dest, uint_farptr_t src, size_t len)
{
    return strncpy(dest, src, len);
}

inline char * strcat_PF(char *dest, uint_farptr_t src)
{
    return strcat(dest, src);
}

inline char * strncat_PF(char *dest, uint_farptr_t src, size_t len)
{
    return strncat(dest, src, len);
}

inline int strcmp_PF(const char *s1, uint_farptr_t s2)
{
    return strcmp(s1, s2);
}

inline int strncmp_PF(const char *s1, uint_farptr_t s2, size_t n)
{
    return strncmp(s1, s2, n);
}

inline int strcasecmp_PF(const char *s1, uint_farptr_t s2)
{
    return stricmp(s1, s2);
}

inline int strncasecmp_PF(const char *s1, uint_farptr_t s2, size_t n)
{
    return strnicmp(s1, s2, n);
}

inline char * strstr_PF(const char *s1, uint_farptr_t s2)
{
    return const_cast<char *>(strstr(s1, s2));
}

inline int memcmp_PF(const void * s1, uint_farptr_t s2, size_t len)
{
    return memcmp(s1, s2, len);
}

// Other APIs for conversions
#define utoa _ultoa
char* dtostrf(double value, char width, uint8_t precision, char* buffer);

#pragma warning( pop )

#endif
