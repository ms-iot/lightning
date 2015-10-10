// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include <ppltasks.h>
#include <concrt.h>
#include "ErrorCodes.h"
#include "DmapSupport.h"

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
    { DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM         , L"The specified device could not be found on the system. Make sure the current driver is DMAP." },
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

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Custom;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;
using namespace Concurrency;
#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)


/**
Get the base address of a memory mapped controller in the SOC.  Exclusive, non-shared
access to the controller is requested.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[out] baseAddress Base address of the controller in question.
\return HRESULT success or error code.
*/
HRESULT GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress)
{
    return GetControllerBaseAddress(deviceName, handle, baseAddress, 0);
}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app
/// Array of CustomDevice references.
/**
This array is used to store persistent references to the CustomDevice references
created for open devices that are exposed by DMap.  This array is only used for
UWP builds.  Holding a reference on the CustomDevices keep them open.  The maximum
number of devices in the array is best kept in agreement with the similar limit
in the DMap driver (16, as of the first release of this software.)
*/
#define MAX_OPEN_DEVICES 16
CustomDevice^ g_devices[MAX_OPEN_DEVICES];
UINT32 g_openDeviceMask = 0;
#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_TOP)

/**
Get the base address of a memory mapped controller in the SOC.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName (zero for UWP build).
\param[out] baseAddress Base address of the controller in question.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return HRESULT success or error code.
*/
HRESULT GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode)
{
    UINT32 controllerAddress = 0;
    HRESULT hr = S_OK;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app

    // If we don't already have the device controller mapped:
    if (baseAddress != nullptr)
    {
        return S_OK; // Initialized already
    }

    Platform::Guid myGuid = Platform::Guid(DMAP_INTERFACE);
    Platform::String^ myAqs = CustomDevice::GetDeviceSelector(myGuid);

    std::wstring inputDeviceString(deviceName);
    size_t subStrLen = inputDeviceString.find(L"{") - 5;
    if (subStrLen < 1)
    {
        return DMAP_E_DMAP_INTERNAL_ERROR;
    }

    // Get substring from after the leading "\\.\" up to the "#{" at start of GUID.
    std::wstring inputDeviceSubstr = inputDeviceString.substr(4, subStrLen);

    // Replace each "#" in the substring with a "\".
    size_t poundPosition;
    while ((poundPosition = inputDeviceSubstr.find(L"#")) != std::wstring::npos)
    {
        inputDeviceSubstr.replace(poundPosition, 1, L"\\");
    }
    // At this point an input device instance name string of:
    //  "\\.\ACPI#INT33FC#1#{109b86ad-f53d-4b76-aa5f-821e2ddf2141}\0"
    // Would be converted to this device instance name string:
    //  "ACPI\INT33FC\1"

    std::shared_ptr<Concurrency::event> findCompleted = std::make_shared<Concurrency::event>();

    auto workItem = ref new WorkItemHandler(
        [&myAqs, &controllerAddress, &inputDeviceSubstr, &handle, &findCompleted, &hr]
    (IAsyncAction^ workItem)
    {
        create_task(DeviceInformation::FindAllAsync(myAqs))
            .then([&controllerAddress, &inputDeviceSubstr, &handle, &findCompleted, &hr]
                (DeviceInformationCollection^ Devices)
        {
            UINT32 mySize = Devices->Size;
            IIterator<DeviceInformation^>^ devIter = Devices->First();
            DeviceInformation^ devInfo;
            bool foundDevice = false;
            Platform::String^ deviceId;
            while (devIter->HasCurrent && !foundDevice)
            {
                UINT32 i;
                devInfo = devIter->Current;
                Platform::String^ devName = devInfo->Name;
                if (devInfo->IsEnabled)
                {
                    IMapView<Platform::String^, Platform::Object^>^ properties = devInfo->Properties;
                    IIterator<IKeyValuePair<Platform::String^, Platform::Object^>^>^ propIter = properties->First();
                    for (i = 0; (i < properties->Size) && !foundDevice; i++)
                    {
                        IKeyValuePair<Platform::String^, Platform::Object^>^ valuePair = propIter->Current;

                        Platform::String^ propKey = valuePair->Key;
                        std::wstring keyStr = propKey->Data();
                        if (_wcsicmp(keyStr.c_str(), L"System.Devices.DeviceInstanceId") == 0)
                        {
                            Platform::Object^ propObj = valuePair->Value;
                            Platform::String^ propStr = propObj->ToString();
                            std::wstring propertiesStr = propStr->Data();

                            if (propertiesStr.compare(inputDeviceSubstr) == 0)
                            {
                                foundDevice = true;
                            }
                        }

                        propIter->MoveNext();
                    }
                }

                devIter->MoveNext();
            } // end - while (devIter->HaveCurrent && !foundDevice)

            if (!foundDevice)
            {
                hr = DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM;
                findCompleted->set();
            }

            if (SUCCEEDED(hr))
            {
                std::wstring devIdStr(devInfo->Id->Data());
                devIdStr.append(L"\\0");
                Platform::String^ devId = ref new Platform::String(devIdStr.c_str());

                create_task(CustomDevice::FromIdAsync(devId, DeviceAccessMode::ReadWrite, DeviceSharingMode::Exclusive))
                    .then([&controllerAddress, &handle, &findCompleted, &hr]
                        (CustomDevice^ device)
                {
                    // Find the first available open device slot.
                    int i = 0;
                    while ((i < MAX_OPEN_DEVICES) && ((g_openDeviceMask & (1 << i)) != 0))
                    {
                        i++;
                    }

                    if (i == MAX_OPEN_DEVICES)
                    {
                        hr = DMAP_E_TOO_MANY_DEVICES_MAPPED;
                        findCompleted->set();
                    }

                    if (SUCCEEDED(hr))
                    {
                        g_devices[i] = device;
                        handle = &g_devices[i];
                        g_openDeviceMask |= 1 << i;

                        IOControlCode^ IOCTL = ref new IOControlCode(0x423, 0x100, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
                        Buffer^ addressBuffer = ref new Buffer(8);

                        create_task(device->SendIOControlAsync(IOCTL, nullptr, addressBuffer))
                            .then([addressBuffer, &controllerAddress, &findCompleted, &hr](UINT32 result)
                        {
                            // We expect a 4-byte address and a 4-byte length to have been transferred 
                            // into the address buffer by the I/O operation.

                            // hr should already be S_OK 
                            // TODO: What happens if result != 8, fail?
                            if (result == 8)
                            {
                                hr = S_OK;
                            }

                            if (SUCCEEDED(hr))
                            {
                                auto reader = DataReader::FromBuffer(addressBuffer);
                                UINT32 address = { 0 };
                                int i;
                                for (i = 0; i < 4; i++)
                                {
                                    address = address | (((UINT32)reader->ReadByte()) << (8 * i));
                                }
                                controllerAddress = address;
                            }

                            findCompleted->set();

                        }); // device->SendIOControlAsync()


                    } // End - if (SUCCEEDED(hr))

                }); // CustomDevice::FromIdAsync()

            } // End - if (SUCCEEDED(hr))

        });  // DeviceInformation::FindAllAsync()

    }); // workItem

    auto asyncAction = ThreadPool::RunAsync(workItem);

    findCompleted->wait();

    baseAddress = (PVOID)controllerAddress;

#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app
    DMAP_MAPMEMORY_OUTPUT_BUFFER buf = { 0 };
    DWORD bytesReturned = 0;

    hr = OpenControllerDevice(deviceName, handle, shareMode);

    if (SUCCEEDED(hr) && (baseAddress == nullptr))
    {
        // Retrieve the base address of controller registers.
        if (!DeviceIoControl(
            handle,
            IOCTL_DMAP_MAPMEMORY,
            nullptr,
            0,
            &buf,
            sizeof(buf),
            &bytesReturned,
            nullptr))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
            baseAddress = nullptr;
        }
        else
        {
            // Pass back the base address of the controller registers to the caller.
            baseAddress = buf.Address;
        }
    }
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

    return hr;
}

/**
Close an open controller device using a method appropriate to how it was opened.
\param[inout] handle Handle to the open controller device.
*/
void DmapCloseController(HANDLE & handle)
{
    if (handle != INVALID_HANDLE_VALUE)
    {

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        CloseHandle(handle);
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        if ((handle >= &g_devices[0]) && (handle < &g_devices[MAX_OPEN_DEVICES]))
        {
            *(CustomDevice^*)handle = nullptr;                  // Close device handle
            UINT32 i = ((CustomDevice^*)handle - g_devices);    // Get index of device in array
            g_openDeviceMask &= ~(1 << i);                      // Indicate device slot is free
        }
#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

        handle = INVALID_HANDLE_VALUE;
    }
}

/**
Open a controller device in the SOC.  This is used for both memory mapped and
IO mapped controllers.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return HRESULT success or error code.
*/
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app
HRESULT OpenControllerDevice(PWCHAR deviceName, HANDLE & handle, DWORD shareMode)
{
    HRESULT hr = S_OK;

    if (handle == INVALID_HANDLE_VALUE)
    {
        // Open the specified device.
        handle = CreateFile(
            deviceName,
            GENERIC_READ | GENERIC_WRITE,
            shareMode,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                hr = E_HANDLE;
            }
        }
    }

    return hr;
}
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app
/**
Acquire an exclusive access lock on a controller.
\param[in] handle Handle opened to the device to be locked.
\return HRESULT success or error code.
*/
HRESULT GetControllerLock(HANDLE & handle)
{
    HRESULT hr = S_OK;
    CustomDevice^ device;
    static IOControlCode^ LockCode = ref new IOControlCode(0x423, 0x103, IOControlAccessMode::Any, IOControlBufferingMethod::Neither);

    if ((handle < &g_devices[0]) || (handle >= &g_devices[MAX_OPEN_DEVICES]))
    {
        hr = DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        device = *(CustomDevice^*)handle;

        std::shared_ptr<Concurrency::event> ioControlCompleted = std::make_shared<Concurrency::event>();

        auto workItem = ref new WorkItemHandler(
            [&device, &hr, &ioControlCompleted](IAsyncAction^ workItem)
        {
            create_task(device->SendIOControlAsync(LockCode, nullptr, nullptr)).then([&ioControlCompleted, &hr](UINT result)
            {
                // TODO: Is this correct?
                hr = result;

                ioControlCompleted->set();
            });
        });

        auto asyncAction = ThreadPool::RunAsync(workItem);

        ioControlCompleted->wait();

    } // End - if (SUCCEEDED(hr))

    return hr;
}
#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app
/**
Release an exclusive access lock on a controller.
\param[in] handle Handle opened to the locked device.
\return HRESULT success or error code.
*/
HRESULT ReleaseControllerLock(HANDLE & handle)
{
    HRESULT hr = S_OK;
    CustomDevice^ device;
    static IOControlCode^ UnlockCode = ref new IOControlCode(0x423, 0x104, IOControlAccessMode::Any, IOControlBufferingMethod::Neither);

    if ((handle < &g_devices[0]) || (handle >= &g_devices[MAX_OPEN_DEVICES]))
    {
        hr = DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED;
    }

    if (SUCCEEDED(hr))
    {
        device = *(CustomDevice^*)handle;

        std::shared_ptr<Concurrency::event> ioControlCompleted = std::make_shared<Concurrency::event>();

        auto workItem = ref new WorkItemHandler(
            [&device, &hr, &ioControlCompleted](IAsyncAction^ workItem)
        {
            create_task(device->SendIOControlAsync(UnlockCode, nullptr, nullptr)).then([&ioControlCompleted, &hr](UINT result)
            {
                // TODO: Is this correct?
                hr = result;

                ioControlCompleted->set();
            });
        });

        auto asyncAction = ThreadPool::RunAsync(workItem);

        ioControlCompleted->wait();

    } // End - if (SUCCEEDED(hr))

    return hr;
}
#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
