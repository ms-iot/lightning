// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "arduino.h"
#ifndef USE_NETWORKSERIAL

void LogLastError()
{
    CHAR *msg;
    DWORD errCode = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
    Log(msg);
}

HardwareSerial::HardwareSerial(const std::wstring &comPort)
    :
    _comHandle(INVALID_HANDLE_VALUE),
    _comPortName(comPort),
    _storageCount(0),
    _storageIndex(0),
    _storageUsed(false),
    _timeout(1000)
{
}

HardwareSerial::~HardwareSerial()
{
    end();
}

HardwareSerial & HardwareSerial::operator= (HardwareSerial &) { return *this; }

DCB HardwareSerial::dcbArray[] =
{
    // sizeof(DCB)
    //             Baudrate at which running 
    //                Binary Mode (skip EOF check)
    //                   Enable parity checking 
    //                      CTS handshaking on output
    //                         DSR handshaking on output  
    //                            DTR Flow control
    //                               DSR Sensitivity
    //                                  Continue TX when Xoff sent
    //                                     Enable output X-ON/X-OFF
    //                                        Enable input X-ON/X-OFF
    //                                           Enable Err Replacement 
    //                                              Enable Null stripping
    //                                                 Rts Flow control
    //                                                    Abort all reads and writes on Error
    //                                                       Reserved
    //                                                          Not currently used
    //                                                             Transmit X-ON threshold
    //                                                                Transmit X-OFF threshold
    //                                                                   Number of bits/byte, 4-8
    //                                                                      0-4=None,Odd,Even,Mark,Space
    //                                                                                0,1,2 = 1, 1.5, 2 
    //                                                                                            Tx and Rx X-ON character
    //                                                                                                Tx and Rx X-OFF character
    //                                                                                                    Error replacement char
    //                                                                                                       End of Input character
    //                                                                                                          Received Event character
    //                                                                                                             Fill for now.
    // SERIAL_5N1                                                                                         
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, NOPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6N1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, NOPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7N1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, NOPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8N1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, NOPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_5N2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, NOPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6N2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, NOPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7N2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, NOPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8N2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, NOPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_5E1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, EVENPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6E1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, EVENPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7E1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, EVENPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8E1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, EVENPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_5E2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, EVENPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6E2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, EVENPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7E2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, EVENPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8E2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, EVENPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_5O1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, ODDPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6O1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, ODDPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7O1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, ODDPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8O1
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, ODDPARITY, ONESTOPBIT, 17, 19, 0, 0, 0, 0 },
    // SERIAL_5O2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, ODDPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_6O2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, ODDPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_7O2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, ODDPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 },
    // SERIAL_8O2
    { sizeof(DCB), 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, ODDPARITY, TWOSTOPBITS, 17, 19, 0, 0, 0, 0 }
};

HardwareSerial::operator bool(void)
{
    if (_comHandle != NULL && _comHandle != INVALID_HANDLE_VALUE)
    {
        return true;
    }
    return false;
}

int HardwareSerial::available(void)
{
    if ((_storageCount - _storageIndex) <= 0)
    {
        peek();
    }
    return _storageCount - _storageIndex;
}

void HardwareSerial::setTimeout(unsigned long timeout)
{
    _timeout = timeout;

    // Sets WIN32 READ/WRITE timeouts
    COMMTIMEOUTS CommTimeouts;
    GetCommTimeouts(_comHandle, &CommTimeouts);

    CommTimeouts.ReadIntervalTimeout = 0;
    CommTimeouts.ReadTotalTimeoutConstant = _timeout;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;

    CommTimeouts.WriteTotalTimeoutConstant = _timeout;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(_comHandle, &CommTimeouts))
    {
#ifdef _DEBUG
        Log("Error %d when setting Com timeouts: ", GetLastError());
        LogLastError();
#endif
    }
}

void HardwareSerial::begin(unsigned long baud, uint8_t config)
{
    DCB dcb = dcbArray[config];

    if ( _comPortName == L"\\\\.\\COM1" )
    {
        g_pins.verifyPinFunction(0, FUNC_SER, GalileoPinsClass::LOCK_FUNCTION);
        g_pins.verifyPinFunction(1, FUNC_SER, GalileoPinsClass::LOCK_FUNCTION);
        pinMode(0, DIRECTION_IN);
        pinMode(1, DIRECTION_OUT);
    }

    _comHandle = CreateFile(_comPortName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);

    if (_comHandle == INVALID_HANDLE_VALUE)
    {
#ifdef _DEBUG
        Log("Error %d due to invalid handle value: ", GetLastError());
        LogLastError();
#endif
        return;
    }

    dcb.BaudRate = baud;

    if (SetCommState(_comHandle, &dcb) == 0)
    {
#ifdef _DEBUG
        Log("Error %d when setting Com state: ", GetLastError());
        LogLastError();
#endif
        return;
    }

    setTimeout(_timeout);
}

void HardwareSerial::end(void)
{
    if (_comHandle != INVALID_HANDLE_VALUE)
    {
        if (CloseHandle(_comHandle) == 0)
        {
            #ifdef _DEBUG
            Log("Error %d when closing Com Handle: ", GetLastError());
            LogLastError();
            #endif
        }
        _comHandle = INVALID_HANDLE_VALUE;
        g_pins.verifyPinFunction(1, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION);
        g_pins.verifyPinFunction(0, FUNC_DIO, GalileoPinsClass::UNLOCK_FUNCTION);
    }
}

void HardwareSerial::flush(void)
{
    if (FlushFileBuffers(_comHandle) == 0)
    {
#ifdef _DEBUG
        Log("Error %d when calling Flush: ", GetLastError());
        LogLastError();
#endif
    }
}

int HardwareSerial::peek(void)
{
    if (!_storageUsed)
    {
        if (!ReadFile(_comHandle, &_storage, 64, &_storageCount, NULL))
        {
#ifdef _DEBUG
            Log("Error %d when reading file: ", GetLastError());
            LogLastError();
#endif
            return -1;
        }

        _storageUsed = true;
        _storageIndex = 0;

        // if nothing was read, return 0 and dont do anything
        if (_storageCount == 0)
        {
            _storageUsed = false;
            return -1;
        }
    }
    return _storage[_storageIndex];
}    

int HardwareSerial::read(void)
{
    if (_storageUsed)
    {
        if (_storageIndex + 1 >= _storageCount)
        {
            _storageUsed = false;
        }
        return _storage[_storageIndex++];
    }
    else
    {
        if (!ReadFile(_comHandle, &_storage, 64, &_storageCount, NULL))
        {
#ifdef _DEBUG
            Log("Error %d when reading file: ", GetLastError());
            LogLastError();
#endif
            return -1;
        }

        _storageUsed = true;
        _storageIndex = 0;

        // If only one character was read, then reset it
        if (_storageIndex + 1 >= _storageCount)
        {
            _storageUsed = false;
        }

        // if nothing was read, return 0 and dont do anything
        if (_storageCount == 0)
        {
            return -1;
        }

        return _storage[_storageIndex++];
    }
}
    
size_t HardwareSerial::write(uint8_t c)
{
    DWORD bytesWritten;

    WriteFile(_comHandle, &c, sizeof(uint8_t), &bytesWritten, NULL);
    return bytesWritten;
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size)
{
    DWORD bytesWritten;

    WriteFile(_comHandle, buffer, size, &bytesWritten, NULL);
    return bytesWritten;
}

HardwareSerial Serial;
HardwareSerial Serial1 = HardwareSerial(L"\\\\.\\COM2");

#endif