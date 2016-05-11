// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include <concrt.h>
#include "ErrorCodes.h"
#include "DmapSupport.h"

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)  // If building a UWP app
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Custom;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;
using namespace Concurrency;

#define WAIT_TIME_MILLIS 10000 // wait up to 10 seconds
#define LEGACY_BUF_SIZE 8  // Required for legacy driver support

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
    void* controllerAddress = 0;
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

    HANDLE findCompleted = CreateEventEx(nullptr, nullptr, 0 /* auto reset */, EVENT_ALL_ACCESS);
    if (findCompleted == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

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
                SetEvent(findCompleted);
                return;
            }

            if (SUCCEEDED(hr))
            {
                std::wstring devIdStr(devInfo->Id->Data());
                devIdStr.append(L"\\0");
                Platform::String^ devId = ref new Platform::String(devIdStr.c_str());

                create_task(CustomDevice::FromIdAsync(devId, DeviceAccessMode::ReadWrite, DeviceSharingMode::Shared))
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
                        SetEvent(findCompleted);
                        return;
                    }

                    if (SUCCEEDED(hr))
                    {
                        g_devices[i] = device;
                        handle = &g_devices[i];
                        g_openDeviceMask |= 1 << i;

                        IOControlCode^ IOCTL = ref new IOControlCode(0x423, 0x100, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);
                        Buffer^ addressBuffer = ref new Buffer(sizeof(DMAP_MAPMEMORY_OUTPUT_BUFFER) + LEGACY_BUF_SIZE);

                        create_task(device->SendIOControlAsync(IOCTL, nullptr, addressBuffer))
                            .then([addressBuffer, &controllerAddress, &findCompleted, &hr](UINT32 result)
                        {
                            // We expect a pointer and a 4-byte length to have been transferred
                            // into the address buffer by the I/O operation. On x86 and ARM,
                            // we should get a 4-byte address + 4-byte length. On AMD64, it's 12 bytes.

                            if (result < sizeof(DMAP_MAPMEMORY_OUTPUT_BUFFER))
                            {
                                hr = E_UNEXPECTED;
                            }
                            else
                            {
                                auto reader = DataReader::FromBuffer(addressBuffer);

                                // The address can be up to 64-bit
                                uint64_t address = 0;

                                for (size_t i = 0; i < sizeof(void*); i++)
                                {
                                    address = address | (((uint64_t)reader->ReadByte()) << (8 * i));
                                }

                                controllerAddress = (void*)address;

                            }

                            SetEvent(findCompleted);

                        }); // device->SendIOControlAsync()

                    } // End - if (SUCCEEDED(hr))

                }); // CustomDevice::FromIdAsync()

            } // End - if (SUCCEEDED(hr))

        });  // DeviceInformation::FindAllAsync()

    }); // workItem

    auto asyncAction = ThreadPool::RunAsync(workItem);

    DWORD dwError = WaitForSingleObjectEx(findCompleted, WAIT_TIME_MILLIS, FALSE);
    if (dwError == WAIT_OBJECT_0)
    {
        baseAddress = controllerAddress;
    }
    else if (dwError == WAIT_TIMEOUT ||
        dwError == WAIT_ABANDONED ||
        dwError == WAIT_IO_COMPLETION)
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(findCompleted);
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
            UINT32 i = (UINT32)((CustomDevice^*)handle - g_devices);    // Get index of device in array
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
Send an IO control code to the controller device driver
\param[in] handle Handle opened to the device
\param[in] iOControlCode The IOControl code to send to the driver
\param[in] bufferToDriver The buffer to send to the driver (nullptr if none)
\param[in] bufferFromDriver The buffer for data from the driver (nullptr if none)
\param[in] timeOutMillis The number of milliseconds to wait for I/O completion (can be INFINITE)
\return HRESULT success or error code.
*/
HRESULT SendIOControlCodeToController(
    HANDLE handle,
    IOControlCode^ iOControlCode,
    IBuffer^ bufferToDriver,
    IBuffer^ bufferFromDriver,
    uint32_t timeOutMillis)
{
    HRESULT hr = S_OK;
    CustomDevice^ device;

    if ((handle < &g_devices[0]) || (handle >= &g_devices[MAX_OPEN_DEVICES]))
    {
        return DMAP_E_INVALID_LOCK_HANDLE_SPECIFIED;
    }

    device = *(CustomDevice^*)handle;

    HANDLE ioControlCompleted = CreateEventEx(nullptr, nullptr, 0 /* auto reset */, EVENT_ALL_ACCESS);
    if (ioControlCompleted == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    auto workItem = ref new WorkItemHandler(
        [device, iOControlCode, ioControlCompleted, &hr, &bufferToDriver, &bufferFromDriver]
    (IAsyncAction^ workItem)
    {
        create_task(device->SendIOControlAsync(iOControlCode, bufferToDriver, bufferFromDriver)).then([&hr, ioControlCompleted](task<unsigned int> t)
        {
            try
            {
                hr = t.get();
            }
            catch (Platform::Exception^ e)
            {
                hr = ERROR_OPERATION_ABORTED;
            }
            catch (...)
            {
                hr = ERROR_OPERATION_ABORTED;
            }
            SetEvent(ioControlCompleted);
        });
    });

    auto asyncAction = ThreadPool::RunAsync(workItem);

    DWORD dwError = WaitForSingleObjectEx(ioControlCompleted, timeOutMillis, FALSE);
    if (dwError == WAIT_OBJECT_0)
    {
        // success
    }
    else if (dwError == WAIT_TIMEOUT ||
        dwError == WAIT_ABANDONED ||
        dwError == WAIT_IO_COMPLETION)
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(ioControlCompleted);
    return hr;
}

/**
Acquire an exclusive access lock on a controller.
\param[in] handle Handle opened to the device to be locked.
\return HRESULT success or error code.
*/
HRESULT GetControllerLock(HANDLE & handle)
{
    static IOControlCode^ LockCode = ref new IOControlCode(0x423, 0x103, IOControlAccessMode::Any, IOControlBufferingMethod::Neither);

    // return SendIOControlCodeToController(handle, LockCode, nullptr, nullptr, WAIT_TIME_MILLIS);
    return S_OK;
}

/**
Release an exclusive access lock on a controller.
\param[in] handle Handle opened to the locked device.
\return HRESULT success or error code.
*/
HRESULT ReleaseControllerLock(HANDLE & handle)
{
    static IOControlCode^ UnlockCode = ref new IOControlCode(0x423, 0x104, IOControlAccessMode::Any, IOControlBufferingMethod::Neither);

    // return SendIOControlCodeToController(handle, UnlockCode, nullptr, nullptr, WAIT_TIME_MILLIS);
    return S_OK;
}
#endif  // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
