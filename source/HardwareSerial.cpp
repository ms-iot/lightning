// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#include "pch.h"

#include "arduino.h"

#ifndef USE_NETWORKSERIAL

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100       // From WinBase.h

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Custom;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;
using namespace Concurrency;

#define SERIAL_READ_LENGTH 1024
#define REQUEST_TIMED_OUT -1
#define NO_PEEK_BYTE REQUEST_TIMED_OUT
#define NUM_READ_OPERATIONS 8
#define RX_TIMEOUT_DURATION 1000000L   // 0.1 seconds
#define TX_TIMEOUT_DURATION 10000000L  // 1 second

#if defined(_M_ARM)
#define UART_DEVICE_NAME "UART0"
#elif defined(_M_IX86) || defined(_M_X64)
#define UART_DEVICE_NAME "UART1"
#endif

#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

void LogLastError()
{
    CHAR *msg;
    DWORD errCode = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
    Log(msg);

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a UWP app:
    HeapFree(GetProcessHeap(), 0, msg);
#endif // !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)   // If building a Win32 app:
    LocalFree(msg);
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
}

/// Constructor.
HardwareSerial::HardwareSerial()
    :
    m_currentReadNumber(0),
    m_queuedReadCount(0),
    m_timeout(1000),
    m_peekByte(NO_PEEK_BYTE),
    m_reader(nullptr),
    m_readThreadCount(0),
    m_cancellationTokenSource(nullptr)
{
    InitializeCriticalSection(&m_readBufferListLock);
}

/// Destructor.
HardwareSerial::~HardwareSerial()
{
    end();
}

HardwareSerial & HardwareSerial::operator= (HardwareSerial &) { return *this; }

HardwareSerial::operator bool(void)
{
    return true;
}

int HardwareSerial::available(void)
{
    std::deque<IBuffer^>::const_iterator iter;
    int bytesAvailable = 0;

    EnterCriticalSection(&m_readBufferListLock);
    iter = m_readBufferList.begin();
    while (iter != m_readBufferList.end())
    {
        bytesAvailable += (*iter)->Length;
        ++iter;
    }
    LeaveCriticalSection(&m_readBufferListLock);

    return bytesAvailable;
}

void HardwareSerial::setTimeout(unsigned long timeout)
{
    m_timeout = timeout;
}

void HardwareSerial::begin(unsigned long baud, uint8_t config)
{
    HRESULT hr = S_OK;

    hr = g_pins.verifyPinFunction(m_txPinNo, FUNC_SER, BoardPinsClass::LOCK_FUNCTION);
    if (FAILED(hr))
    {
        ThrowError(hr, "An error occurred configuring pin %d as for Serial TX use.", m_txPinNo);
    }

    hr = g_pins.verifyPinFunction(m_rxPinNo, FUNC_SER, BoardPinsClass::LOCK_FUNCTION);
    if (FAILED(hr))
    {
        ThrowError(hr, "An error occurred configuring pin %d for Serial RX use.", m_rxPinNo);
    }

    // Set up objects needed to cancel read threads when .end() is called.
    m_cancellationTokenSource = new Concurrency::cancellation_token_source();
    Concurrency::cancellation_token cToken(m_cancellationTokenSource->get_token());

    m_serialConfig = (SerialConfigs)config;
    m_baudRate = baud;

    m_currentReadNumber = 0;
    m_queuedReadCount = 0;
    m_peekByte = NO_PEEK_BYTE;
    m_readThreadCount = 0;

    // Create event we use to determine when the UART open operation has completed.  This is used,
    // rather than .wait() becaue .begin() may be called from the main thread when an object in a 
    // library is constructed--and neither .wait() nor .get() can be called from the main thread.
    std::shared_ptr<Concurrency::event> findCompleted = std::make_shared<Concurrency::event>();

    auto workItem = ref new WorkItemHandler(
        [this, &hr, &findCompleted]
    (IAsyncAction^ workItem)
    {
        OpenUart(hr, findCompleted);
    }); // workItem

        // Run the async operation to open the UART.
    auto asyncAction = ThreadPool::RunAsync(workItem);

    // Wait for the UART open operation to complete.
    findCompleted->wait();

    if (SUCCEEDED(hr))
    {
        // setup our data writer for handling outgoing data
        m_dataWriter = ref new Windows::Storage::Streams::DataWriter(m_serialDevice->OutputStream);

        // Queue up 8 read operations.
        for (int i = 0; i < NUM_READ_OPERATIONS; i++)
        {
            Listen(cToken);
        }
    }

    if (FAILED(hr))
    {
        ThrowError(hr, "An error occurred attempting to access the UART.");
    }
}

void HardwareSerial::Listen(Concurrency::cancellation_token cToken)
{
    IBuffer^ newBuffer = ref new Buffer(SERIAL_READ_LENGTH);
    if (newBuffer == nullptr)
    {
        return;
    }

    EnterCriticalSection(&m_readBufferListLock);
    uint32_t serialNo = m_queuedReadCount;
    m_queuedReadCount++;
    m_readThreadCount++;
    LeaveCriticalSection(&m_readBufferListLock);

    concurrency::create_task(m_serialDevice->InputStream->ReadAsync(newBuffer, SERIAL_READ_LENGTH, InputStreamOptions::None), cToken)
        .then([this, serialNo, cToken](IBuffer^ buffer)
    {
        if (!cToken.is_canceled())
        {
            // Wait for any reads prior to this one that have not completed yet.
            BOOL bufferIsCurrent = FALSE;
            do
            {
                EnterCriticalSection(&m_readBufferListLock);
                if (serialNo == m_currentReadNumber)
                {
                    bufferIsCurrent = TRUE;
                    m_readBufferList.push_back(buffer);
                    m_currentReadNumber++;
                }
                LeaveCriticalSection(&m_readBufferListLock);
                if (!bufferIsCurrent)
                {
                    Sleep(0);
                }
            } while (!bufferIsCurrent && !cToken.is_canceled());

        }
    })
        .then([this, cToken](concurrency::task<void> t)
    {
        // This block of code executes whenever the ReadAsync() operation goes away--whether it
        // completed or was cancelled.
        EnterCriticalSection(&m_readBufferListLock);
        m_readThreadCount--;
        LeaveCriticalSection(&m_readBufferListLock);
        try
        {
            // This trows an exception if the operation was cancelled.
            t.get();

            // If we get here, the operation was not cancelled.  Queue up another read operation.
            Listen(cToken);
        }
        catch (concurrency::task_canceled)
        {
            // If the operation was cancelled, ignore the exception.
        }
    });
}

/// Find the UART serial device, open it, and configure it.
void HardwareSerial::OpenUart(HRESULT& hr, std::shared_ptr<Concurrency::event>& findCompleted)
{
    m_deviceInformation = nullptr;
    hr = S_OK;
    //
    // Get a list of serial devices available on this device
    //
    Concurrency::create_task(ListAvailableSerialDevicesAsync(), m_cancellationTokenSource->get_token())
        .then([this, &hr, &findCompleted](DeviceInformationCollection ^serialDeviceCollection)
    {
        if (m_cancellationTokenSource->get_token().is_canceled())
        {
            hr = E_ABORT;
            findCompleted->set();
        }
        else
        {
            IIterator<DeviceInformation^>^ devIter = serialDeviceCollection->First();
            if (devIter->HasCurrent)
            {
                m_deviceInformation = devIter->Current;
            }
            else
            {
                hr = E_NOINTERFACE;
                findCompleted->set();
            }
        }

        return;
    })
        .then([this, &hr, &findCompleted]()
    {
        if (SUCCEEDED(hr))
        {
            if (m_cancellationTokenSource->get_token().is_canceled())
            {
                hr = E_ABORT;
                findCompleted->set();
            }
            else
            {
                //
                // Open the serial device we found.
                //
                Concurrency::create_task(
                    SerialDevice::FromIdAsync(m_deviceInformation->Id),
                    m_cancellationTokenSource->get_token())
                    .then([this, &hr, &findCompleted](SerialDevice ^serial_device)
                {
                    if (m_cancellationTokenSource->get_token().is_canceled())
                    {
                        hr = E_ABORT;
                        findCompleted->set();
                    }
                    else
                    {
                        //
                        // Configure the serial device.
                        //
                        Windows::Foundation::TimeSpan _rxTimeOut;
                        Windows::Foundation::TimeSpan _txTimeOut;
                        _rxTimeOut.Duration = RX_TIMEOUT_DURATION;
                        _txTimeOut.Duration = TX_TIMEOUT_DURATION;
                        m_serialDevice = serial_device;

                        hr = ConfigureSerialSettings(m_serialConfig, m_serialDevice);

                        if (FAILED(hr))
                        {
                            findCompleted->set();
                        }
                        else // if (SUCCEEDED(hr))
                        {
                            try
                            {
                                m_serialDevice->WriteTimeout = _txTimeOut;
                                m_serialDevice->ReadTimeout = _rxTimeOut;
                                m_serialDevice->BaudRate = m_baudRate;
                                m_serialDevice->Handshake = SerialHandshake::None;
                            }
                            catch (Platform::Exception ^ex)
                            {
                                hr = E_FAIL;
                                CloseUart();
                                findCompleted->set();
                            }
                        } // if (SUCCEEDED(hr)
                    }

                    // On success, set the findCompleted event.  On error it is set where the error is flagged,
                    // so we can't set it again here--the variable may already be gone.
                    if (SUCCEEDED(hr))
                    {
                        findCompleted->set();
                    }
                    return;
                });
                return;
            } // if (m_cancellationTokenSource->get_token().is_canceled()) else
        } // if (SUCCEEDED(hr))
    });
}

/// <summary>
/// Close the comport currently connected
/// </summary
void HardwareSerial::CloseUart(void)
{
    if (m_cancellationTokenSource != nullptr)
    {
        m_cancellationTokenSource->cancel();
    }

    while (m_readThreadCount > 0)
    {
        Sleep(0);
    }

    delete(m_reader);
    m_reader = nullptr;

    delete(m_dataWriter);
    m_dataWriter = nullptr;

    delete(m_serialDevice);
    m_serialDevice = nullptr;

    delete(m_cancellationTokenSource);
    m_cancellationTokenSource = nullptr;

    m_readBufferList.clear();
}

/// <summary>
/// An asynchronous operation that returns a collection of DeviceInformation objects for all serial devices detected on the device.
/// </summary>
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^HardwareSerial::ListAvailableSerialDevicesAsync(void)
{
    // Construct AQS String for all serial devices on system
    Platform::String ^serialDevices_aqs = SerialDevice::GetDeviceSelector(UART_DEVICE_NAME);

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(serialDevices_aqs);
}

/// Configure the serial port with the desired data format settings.
HRESULT HardwareSerial::ConfigureSerialSettings(SerialConfigs configs, SerialDevice^ device)
{
    HRESULT hr = S_OK;

    switch (m_serialConfig)
    {
    case SERIAL_5N1:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_6N1:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_7N1:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_8N1:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_5N2:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_6N2:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_7N2:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_8N2:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::None;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_5E1:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_6E1:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_7E1:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_8E1:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_5E2:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_6E2:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_7E2:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_8E2:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::Even;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_5O1:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_6O1:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_7O1:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_8O1:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::One;
        break;
    case SERIAL_5O2:
        m_serialDevice->DataBits = 5;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_6O2:
        m_serialDevice->DataBits = 6;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_7O2:
        m_serialDevice->DataBits = 7;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    case SERIAL_8O2:
        m_serialDevice->DataBits = 8;
        m_serialDevice->Parity = SerialParity::Odd;
        m_serialDevice->StopBits = SerialStopBitCount::Two;
        break;
    default:
        hr = E_INVALIDARG;
    }

    return hr;
}

/// Free the serial device and associated resources on a best effort basis.
void HardwareSerial::end(void)
{
    CloseUart();

    g_pins.verifyPinFunction(m_txPinNo, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
    g_pins.verifyPinFunction(m_rxPinNo, FUNC_DIO, BoardPinsClass::UNLOCK_FUNCTION);
}

void HardwareSerial::flush(void)
{
}

int HardwareSerial::peek(void)
{
    int returnByte = NO_PEEK_BYTE;

    EnterCriticalSection(&m_readBufferListLock);
    if (m_peekByte == NO_PEEK_BYTE)
    {
        m_peekByte = read();
    }
    returnByte = m_peekByte;
    LeaveCriticalSection(&m_readBufferListLock);

    return returnByte;
}

int HardwareSerial::read(void)
{
    IBuffer^ buffer;
    int returnByte = NO_PEEK_BYTE;

    EnterCriticalSection(&m_readBufferListLock);

    if (m_peekByte == NO_PEEK_BYTE)
    {
        while ((m_reader == nullptr) && (m_readBufferList.size() > 0))
        {
            buffer = m_readBufferList.front();
            m_reader = DataReader::FromBuffer(buffer);

            if (m_reader->UnconsumedBufferLength == 0)
            {
                m_readBufferList.pop_front();
                m_reader = nullptr;
            }
        }

        if (m_reader != nullptr)
        {
            returnByte = (int)m_reader->ReadByte();

            if (m_reader->UnconsumedBufferLength == 0)
            {
                m_readBufferList.pop_front();
                m_reader = nullptr;
            }
        }
    }
    else
    {
        returnByte = m_peekByte;
        m_peekByte = NO_PEEK_BYTE;
    }

    LeaveCriticalSection(&m_readBufferListLock);

    return returnByte;
}

size_t HardwareSerial::write(uint8_t c)
{
    m_dataWriter->WriteByte(c);

    concurrency::create_task(m_dataWriter->StoreAsync())
        .wait();

    return 1;
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size)
{
    Platform::Array<uint8_t>^ dataArray = ref new Platform::Array<uint8_t>((UINT)size);
    for (size_t i = 0; i < size; i++)
    {
        dataArray[(UINT)i] = buffer[i];
    }

    m_dataWriter->WriteBytes(dataArray);

    concurrency::create_task(m_dataWriter->StoreAsync(), m_cancellationTokenSource->get_token())
        .wait();

    return size;
}

HardwareSerial Serial;

#endif