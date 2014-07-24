#include "WString.h"

#include <algorithm>
#include <cstring>
#include <string>

String::String (
    void
) {}

String::String (
    const char c_
) {
    _buffer = c_;
}

String::String (
    const char *c_str_
) {
    _buffer = c_str_;
}

String::String (
    const String &s_
) {
    _buffer = s_._buffer;
}

String::~String (
    void
) {}

String &
String::operator= (
    const char *c_str_
) {
    _buffer = c_str_;
    return *this;
}

String &
String::operator= (
    const String &s_
) {
    _buffer = s_._buffer;
    return *this;
}

char &
String::operator[] (
    const unsigned int n_
) {
    return _buffer[n_];
}

char
String::operator[] (
    const unsigned int n_
) const {
    return _buffer[n_];
}

String
String::operator+ (
    const String &s_
) const {
    return ((_buffer + s_._buffer).c_str());
}

bool
String::operator== (
    const String &s_
) const {
    return !(_buffer.compare(s_._buffer));
}

char
String::charAt (
    const unsigned int n_
) const {
    return _buffer.at(n_);
}

int
String::compareTo (
    const String &s_
) const {
    return (_buffer.compare(s_._buffer));
}

String
String::concat (
    const String &s_
) const {
    return ((*this) + s_);
}

bool
String::endsWith (
    const String &s_
) const {
    bool result(false);

    if ((*this).length() < s_.length()) { return false; }

    auto t_c = (*this)._buffer.crbegin();
    auto s_end = s_._buffer.crend();
    for (auto s_c = s_._buffer.crbegin(); s_c != s_end; ++s_c, ++t_c) {
        result = (*t_c == *s_c);
    }

    return result;
}

bool
String::equals (
    const String &s_
) const {
    return ((*this) == s_);
}

bool
String::equalsIgnoreCase (
    const String &s_
) const {
    std::string a, b;

    transform(_buffer.begin(), _buffer.end(), a.begin(), ::tolower);
    transform(s_._buffer.begin(), s_._buffer.end(), b.begin(), ::tolower);

    return !(a.compare(b));
}

size_t
String::getBytes (
    byte * const buffer_,
    const size_t length_
) const {
    strncpy_s(reinterpret_cast<char *>(buffer_), length_, _buffer.c_str(), _TRUNCATE);
    return strlen(reinterpret_cast<char *>(buffer_));
}

size_t
String::indexOf (
    const String &val_,
    const size_t from_
) const {
    size_t position = _buffer.find(val_._buffer, from_);
    return (((position == std::string::npos) * -1) + ((position != std::string::npos) * position));
}

size_t
String::lastIndexOf (
    const String &val_,
    const size_t from_
) const {
    size_t position = _buffer.rfind(val_._buffer, from_);
    return (((position == std::string::npos) * -1) + ((position != std::string::npos) * position));
}

size_t
String::length (
    void
) const {
    return _buffer.length();
}

String
String::replace (
    const String &from_,
    const String &to_
) const {
    size_t i(0);
    std::string replaced = _buffer;

    while (std::string::npos != (i = replaced.find(from_._buffer))) {
        replaced.replace(i, from_._buffer.length(), to_._buffer);
    }

    return replaced.c_str();
}

void
String::reserve (
    const size_t size_
) {
    _buffer.reserve(size_);
}

String &
String::setCharAt (
    const size_t index_,
    const char c_
) {
    _buffer[index_] = c_;
    return (*this);
}

bool
String::startsWith (
    const String &s_
) const {
    bool result(false);

    if ((*this).length() < s_.length()) { return false; }

    auto t_c = (*this)._buffer.cbegin();
    auto s_end = s_._buffer.cend();
    for (auto s_c = s_._buffer.cbegin(); s_c != s_end; ++s_c, ++t_c) {
        result = (*t_c == *s_c);
    }

    return result;
}

String
String::substring (
    const size_t from_,
    const size_t to_
) const {
    return ((_buffer.substr(from_, to_)).c_str());
}

size_t
String::toCharArray (
    char * const buffer_,
    const size_t length_
) const {
    strncpy_s(reinterpret_cast<char *>(buffer_), length_, _buffer.c_str(), _TRUNCATE);
    return strlen(reinterpret_cast<char *>(buffer_));
}

long int
String::toInt (
    void
) const {
    return atol(_buffer.c_str());
}

String &
String::toLowerCase (
    void
) {
    std::transform(_buffer.begin(), _buffer.end(), _buffer.begin(), ::tolower);
    return (*this);
}

String &
String::toUpperCase (
    void
) {
    std::transform(_buffer.begin(), _buffer.end(), _buffer.begin(), ::toupper);
    return (*this);
}

String &
String::trim (
    void
) {
    char *trimmed_string = const_cast<char *>(_buffer.c_str());
    char *end = const_cast<char *>((trimmed_string + (_buffer.length() - 1)));

    while (*trimmed_string == ' ') { ++trimmed_string; }
    while (*end == ' ') { --end; }
    *(end + 1) = '\0';

    _buffer = trimmed_string;

    return (*this);
}


  /**************************/
 /* Friend implementations */
/**************************/

std::ostream &
operator<< (
    std::ostream &os_,
    const String &s_
) {
    std::cout << s_._buffer;
    return os_;
}
