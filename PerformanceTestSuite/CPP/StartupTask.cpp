// Background application that sets a GPIO pin high and low as fast as possible

#include "pch.h"
#include "StartupTask.h"

using namespace GpioPerformanceTestCPP;
using namespace Microsoft::IoT::Lightning::Providers;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;

static const int LED_PIN = 5;

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    bool hasLightning = LightningProvider::IsLightningEnabled;
    if (hasLightning)
    {
        // Set Lightning as the default provider
        LowLevelDevicesController::DefaultProvider = LightningProvider::GetAggregateProvider();
    }
    else
    {
        OutputDebugString(L"Lightning NOT supported!\n");
    }

    auto gpio = GpioController::GetDefault();
    if (gpio == nullptr)
    {
        OutputDebugString(L"No GPIO controller found.\n");
        return;
    }
    Windows::Devices::Gpio::GpioPin ^pin = gpio->OpenPin(LED_PIN);
    pin->SetDriveMode(GpioPinDriveMode::Output);
    for (;;)
    {
        pin->Write(GpioPinValue::High);
        pin->Write(GpioPinValue::Low);
    }
}
