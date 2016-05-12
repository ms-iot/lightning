// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "ErrorCodes.h"
#include <strsafe.h>

std::map<HRESULT, LPCWSTR> DmapErrors = {

    { DMAP_E_PIN_FUNCTION_LOCKED                , L"A pin is already locked for use for a function that conflicts with the use requested." },
    { DMAP_E_PIN_NUMBER_TOO_LARGE_FOR_BOARD     , L"A pin number was specified that is beyond the range of pins supported by the board." },
    { DMAP_E_FUNCTION_NOT_SUPPORTED_ON_PIN      , L"A function has been requested on a pin that does not support that function." },
    { DMAP_E_INVALID_PIN_DIRECTION              , L"A pin direction was specified that was neither INPUT nor OUPUT." },
    { DMAP_E_DMAP_INTERNAL_ERROR                , L"An internal inconsistency in the DMap code has been found." },
    { DMAP_E_INVALID_PIN_STATE_SPECIFIED        , L"A desited state for a pin was specified that was neither HIGH nor LOW." },
    { DMAP_E_BOARD_TYPE_NOT_RECOGNIZED          , L"The board type could not be determined." },
    { DMAP_E_INVALID_BOARD_TYPE_SPECIFIED       , L"An invalid board type was specified." },
    { DMAP_E_INVALID_PORT_BIT_FOR_DEVICE        , L"The port/bit specified does not exist on the device." },
    { DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED      , L"An invalid handle was specified attempting to get a controller lock." },
    { DMAP_E_TOO_MANY_DEVICES_MAPPED            , L"An attempt was made to map more than the maximum number of devices." },
    { DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM         , L"The specified device could not be found on the system. Please Make sure the Lightning driver is enabled. For more information refer to the Lightning Setup Guide: http://ms-iot.github.io/content/en-US/win10/LightningSetup.htm" },
    { DMAP_E_I2C_ADDRESS_OUT_OF_RANGE           , L"The specified I2C address is outside the legal range for 7-bit I2C addresses." },
    { DMAP_E_I2C_NO_OR_EMPTY_WRITE_BUFFER       , L"None or empty, write buffer was specified." },
    { DMAP_E_I2C_NO_OR_ZERO_LENGTH_READ_BUFFER  , L"None or zero length, read buffer was specified." },
    { DMAP_E_I2C_NO_CALLBACK_ROUTINE_SPECIFIED  , L"No callback routine was specified to be queued." },
    { DMAP_E_I2C_BUS_LOCK_TIMEOUT               , L"More than 5 seconds elapsed waiting to acquire the I2C bus lock." },
    { DMAP_E_I2C_READ_INCOMPLETE                , L"Fewer than the expected number of bytes were received on the I2C bus." },
    { DMAP_E_I2C_EXTRA_DATA_RECEIVED            , L"More than the expected number of bytes were received on the I2C bus." },
    { DMAP_E_I2C_OPERATION_INCOMPLETE           , L"One or more transfers remained undone at the end of the I2C operation." },
    { DMAP_E_I2C_INVALID_BUS_NUMBER_SPECIFIED   , L"The I2C bus specified does not exist." },
    { DMAP_E_I2C_TRANSFER_LENGTH_OVER_MAX       , L"The specified I2C transfer length is longer than the controller supports." },
    { DMAP_E_ADC_DATA_FROM_WRONG_CHANNEL        , L"ADC data for a different channel than requested was received." },
    { DMAP_E_ADC_DOES_NOT_HAVE_REQUESTED_CHANNEL, L"The ADC does not have the channel that has been requested." },
    { DMAP_E_SPI_DATA_WIDTH_MISMATCH            , L"The width of data sent does not match the data width set on the SPI controller." },
    { DMAP_E_SPI_BUS_REQUESTED_DOES_NOT_EXIST   , L"The specified BUS number does not exist on this board." },
    { DMAP_E_SPI_MODE_SPECIFIED_IS_INVALID      , L"The SPI mode specified is not a legal SPI mode value (0-3)." },
    { DMAP_E_SPI_SPEED_SPECIFIED_IS_INVALID     , L"The SPI speed specified is not in the supported range." },
    { DMAP_E_SPI_BUFFER_TRANSFER_NOT_IMPLEMENTED, L"This SPI implementation does not support buffer transfers." },
    { DMAP_E_SPI_DATA_WIDTH_SPECIFIED_IS_INVALID, L"The specified number of bits per transfer is not supported by the SPI controller." },
    { DMAP_E_GPIO_PIN_IS_SET_TO_PWM             , L"A GPIO operation was performed on a pin configured as a PWM output." }
};

LIGHTNING_DLL_API void ThrowError(_In_ HRESULT hr, _In_ _Printf_format_string_ STRSAFE_LPCSTR pszFormat, ...)
{

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
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
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
    int result;
    char buf[BUFSIZ];
    wchar_t wbuf[BUFSIZ];
    va_list argList;
    va_start(argList, pszFormat);
    result = vsprintf_s(buf, BUFSIZ, pszFormat, argList);

    va_end(argList);

    if (result > 0)
    {
        MultiByteToWideChar(CP_ACP, 0, buf, _countof(buf), wbuf, _countof(wbuf));
        auto it = DmapErrors.find(hr);
        Platform::String^ exceptionString;
        if (it != DmapErrors.end())
        {
            std::wstring exceptionSz(wbuf);
            exceptionSz.append(L": ");
            exceptionSz.append(it->second);
            exceptionString = ref new Platform::String(exceptionSz.c_str());
        }
        else
        {
            exceptionString = ref new Platform::String(wbuf);
        }
        throw ref new Platform::Exception(hr, exceptionString);
    }
    else
    {
        throw ref new Platform::Exception(hr);
    }
#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
LIGHTNING_DLL_API void ThrowError(_In_ HRESULT hr, _In_ _Printf_format_string_ STRSAFE_LPCWSTR pszFormat, ...)
{
    int result;
    wchar_t wbuf[BUFSIZ];
    va_list argList;
    va_start(argList, pszFormat);
    result = vswprintf_s(wbuf, BUFSIZ, pszFormat, argList);

    va_end(argList);

    if (result > 0)
    {
        auto it = DmapErrors.find(hr);
        Platform::String^ exceptionString;
        if (it != DmapErrors.end())
        {
            std::wstring exceptionSz(wbuf);
            exceptionSz.append(L"\n");
            exceptionSz.append(it->second);
            exceptionString = ref new Platform::String(exceptionSz.c_str());
        }
        else
        {
            exceptionString = ref new Platform::String(wbuf);
        }
        throw ref new Platform::Exception(hr, exceptionString);
    }
    else
    {
        throw ref new Platform::Exception(hr);
    }
}
#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
