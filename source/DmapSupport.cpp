// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include <windows.h>
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a UWP app
#include "pch.h"
#endif	// !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <ppltasks.h>

#include "ErrorCodes.h"
#include "DmapSupport.h"


#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a UWP app
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Custom;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Concurrency;
#endif	// !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

//void GetControllersAsyncCompletedHandler(
//	Windows::Foundation::IAsyncOperation<CustomDevice^> ^operation,
//	Windows::Foundation::AsyncStatus status);
//
//void GetControllerAddressCompletedHandler(
//	Windows::Foundation::IAsyncOperation<ULONG32> ^operation,
//	Windows::Foundation::AsyncStatus status);

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

/**
Get the base address of a memory mapped controller in the SOC.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[out] baseAddress Base address of the controller in question.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return HRESULT success or error code.
*/

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a UWP app
CustomDevice^ g_device;
#endif	// !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_TOP)

// Test routine to open the DMap driver.
HRESULT GetControllerBaseAddress(PWCHAR deviceName, HANDLE & handle, PVOID & baseAddress, DWORD shareMode)
{
	UINT32 controllerAddress = 0;
	HRESULT hr = S_OK;

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a UWP app
	UNREFERENCED_PARAMETER(handle);

	Platform::Guid myGuid = Platform::Guid({ 0x109b86ad, 0xf53d, 0x4b76, 0xaa, 0x5f, 0x82, 0x1e, 0x2d, 0xdf, 0x21, 0x41 });
	Platform::String^ myAqs = CustomDevice::GetDeviceSelector(myGuid);

	create_task(DeviceInformation::FindAllAsync(myAqs)).then([&controllerAddress](DeviceInformationCollection^ Devices)
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
			//			std::wstring devNameStr(devName->Data());
			//			Platform::String^ devId = devInfo->Id;
			//			std::wstring devIdStr(devId->Data());
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

						if (_wcsicmp(propertiesStr.c_str(), L"ACPI\\INT33FC\\1") == 0)
						{
							//							deviceId = devInfo->Id;
							foundDevice = true;
						}
					}

					propIter->MoveNext();
				}
			}

			devIter->MoveNext();
		} // end - while

		if (foundDevice)
		{
			std::wstring devIdStr(devInfo->Id->Data());
			devIdStr.append(L"\\0");
			Platform::String^ devId = ref new Platform::String(devIdStr.c_str());

			create_task(CustomDevice::FromIdAsync(devId, DeviceAccessMode::ReadWrite, DeviceSharingMode::Exclusive))
				.then([&controllerAddress](CustomDevice^ device)
			{
				g_device = device;
				IOControlCode^ IOCTL = ref new IOControlCode(0x423, 0x100, IOControlAccessMode::Any, IOControlBufferingMethod::Buffered);

				IAsyncOperation<ULONG32>^ getAddressOp;
				Buffer^ addressBuffer = ref new Buffer(8);

				create_task(device->SendIOControlAsync(IOCTL, nullptr, addressBuffer))
					.then([&addressBuffer, &controllerAddress](UINT32 result)
				{
					auto reader = DataReader::FromBuffer(addressBuffer);
					UINT32 address = { 0 };
					int i;
					for (i = 0; i < 4; i++)
					{
						address = address | (((UINT32)reader->ReadByte()) << (8 * i));
					}
					controllerAddress = address;
					//UINT32 length = { 0 };
					//for (i = 0; i < 4; i++)
					//{
					//	length = length | (((UINT32)reader->ReadByte()) << (8 * i));
					//}
					//UINT32 mappingLength = length;

					return;
				}).wait();

				return;
			}).wait();

		}
		return;
	}).wait();

	baseAddress = (PVOID)controllerAddress;
#endif	// !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a Win32 app
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
Open a controller device in the SOC.  This is used for both memory mapped and
IO mapped controllers.
\param[in] deviceName The name of the PCI device used to map the controller in question.
\param[out] handle Handle opened to the device specified by deviceName.
\param[in] shareMode Sharing specifier as specified to CreateFile().
\return HRESULT success or error code.
*/
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)	// If building a Win32 app
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
