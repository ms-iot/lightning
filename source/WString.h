// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef WString_H
#define WString_H

#include <iostream>

#ifndef byte
typedef unsigned char byte;
#endif

// The String class, part of the core as of version 0019,
// allows you to use and manipulate strings of text in more
// complex ways than character arrays do. You can concatenate
// Strings, append to them, search for and replace substrings,
// and more. It takes more memory than a simple character
// array, but it is also more useful. For reference, character
// arrays are referred to as strings with a small s, and
// instances of the String class are referred to as Strings
// with a capital S.Note that constant strings, specified in
// "double quotes" are treated as char arrays, not instances
// of the String class.
class String {
  public:
	// Definition(s)
	// Constructor and destructor method(s)
    String (
        void
    );

    String (
        const char c_
    );
	
    String (
        const char *c_str_
    );
	
    String (
        const String &s_
    );
	
	virtual
    ~String (
        void
    );

	// Friends list
	friend
    std::ostream &
    operator<< (
        std::ostream &os_,
        const String &s_
    );
	
	// Public instance variable(s)
	// Operator override method(s)
    String &
    operator= (
        const char *c_str_
    );
	
    String &
    operator= (
        const String &s_
    );
	
	// Allows you access to the individual characters of a string.
	char &
    operator[] (
        const unsigned int n_
    );
	
    // Allows you access to the individual characters of a string.
    char
    operator[] (
        const unsigned int n_
    ) const;

    // Combines, or concatenates two strings into one new String.
    // The second string is appended to the first, and the result
    // is placed in a new String. Works the same as string.concat().
    String
    operator+ (
        const String &s_
    ) const;

    // Compares two strings for equality.The comparison is
    // case-sensitive, meaning the String "hello" is not equal
    // to the String "HELLO".Functionally the same as string.equals()
    bool
    operator== (
        const String &s_
    ) const;

	// Public method(s)
	
	// Access a particular character of the String.
	char
    charAt (
        const unsigned int n_
    ) const;
	
    // Compares two Strings, testing whether one comes before
    // or after the other, or whether they're equal. The strings
    // are compared character by character, using the ASCII
    // values of the characters. That means, for example, that 'a'
    // comes before 'b' but after 'A'. Numbers come before letters.
    int
    compareTo (
        const String &s_
    ) const;

    // Combines, or concatenates two strings into one new String.
    // The second string is appended to the first, and the result
    // is placed in a new String.
    String
    concat (
        const String &s_
    ) const;

    // Tests whether or not a String ends with the characters of another String. 
	bool
    endsWith (
        const String &s_
    ) const;

    // Compares two strings for equality. The comparison is
    // case-sensitive, meaning the String("hello") is not equal
    // to the String("HELLO").
	bool
    equals (
        const String &s_
    ) const;

    // Compares two strings for equality. The comparison is
    // not case-sensitive, meaning the String("hello") is equal
    // to the String("HELLO").
	bool
    equalsIgnoreCase (
        const String &s_
    ) const;

	// Copies the string's characters to the supplied buffer.
    size_t
    getBytes (
        byte * const buffer_,
        const size_t length_
    ) const;

	// Locates a character or String within another String.
    // By default, searches from the beginning of the String,
    // but can also start from a given index, allowing for the
    // locating of all instances of the character or String.
    size_t
    indexOf (
        const String &val_,
        const size_t from_ = 0
    ) const;

    // Locates a character or String within another String.
    // By default, searches from the end of the String, but
    // can also work backwards from a given index, allowing
    // for the locating of all instances of the character or String
	size_t
    lastIndexOf (
        const String &val_,
        const size_t from_ = std::string::npos
    ) const;

    // Returns the length of the String, in characters.
    // (Note that this doesn't include a trailing null character.)
    size_t
    length (
        void
    ) const;

    // The String replace() function allows you to replace
    // all instances of a given character with another character.
    // You can also use replace to replace substrings of a string
    // with a different substring.
	String
    replace (
        const String &from_,
        const String &to_
    ) const;

	// The String reserve() function allows you to allocate
    // a buffer in memory for manipulating strings.
    void
    reserve (
        const size_t size_
    );

    // Sets a character of the String. Has no effect on
    // indices outside the existing length of the String.
    String &
    setCharAt (
        const size_t index_,
        const char c_
    );

	// Tests whether or not a String starts with the characters
    // of another String.
    bool
    startsWith (
        const String &s_
    ) const;

    // Get a substring of a String. The starting index is inclusive
    // (the corresponding character is included in the substring),
    // but the optional ending index is exclusive (the corresponding
    // character is not included in the substring). If the ending index
    // is omitted, the substring continues to the end of the String.
	String
    substring (
        const size_t from_,
        const size_t to_ = std::string::npos
    ) const;

    // Copies the string's characters to the supplied buffer.
	size_t
    toCharArray (
        char * const buffer_,
        const size_t length_
    ) const;

    // Converts a valid String to an integer. The input string
    // should start with an integral number. If the string contains
    // non-integral numbers, the function will stop performing the
    // conversion.
	long int
    toInt (
        void
    ) const;

    // Get a lower-case version of a String. As of 1.0, toLowerCase()
    // modifies the string in place rather than returning a new one.
	String &
    toLowerCase (
        void
    );

    // Get an upper-case version of a String. As of 1.0, toUpperCase()
    // modifies the string in place rather than returning a new one.
	String &
    toUpperCase (
        void
    );

    // Get a version of the String with any leading and trailing
    // whitespace removed. As of 1.0, trim() modifies the string
    // in place rather than returning a new one.
	String &
    trim (
        void
    );

  private:
	// Private instance variable(s)	
	std::string _buffer;
	// Private method(s)
};

#endif
