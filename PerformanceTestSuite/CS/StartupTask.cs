using Microsoft.IoT.Lightning.Providers;
using System.Diagnostics;
using Windows.ApplicationModel.Background;
using Windows.Devices;
using Windows.Devices.Gpio;

// Background application that sets a GPIO pin high and low as fast as possible

namespace GpioPerformanceTestCS
{
    public sealed class StartupTask : IBackgroundTask
    {
        private const int LED_PIN = 5;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            if (LightningProvider.IsLightningEnabled)
            {
                LowLevelDevicesController.DefaultProvider = LightningProvider.GetAggregateProvider();
            }
            else
            {
                Debug.WriteLine("Lightning NOT supported!");
            }

            var gpioController = GpioController.GetDefault();
            if (gpioController == null)
            {
                Debug.WriteLine("No GPIO Controller found.");
                return;
            }

            var pin = gpioController.OpenPin(LED_PIN);
            pin.SetDriveMode(GpioPinDriveMode.Output);
            for (;;)
            {
                pin.Write(GpioPinValue.High);
                pin.Write(GpioPinValue.Low);
            }
        }
    }
}
