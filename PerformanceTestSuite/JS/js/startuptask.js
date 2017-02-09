/*eslint no-constant-condition: ["error", { "checkLoops": false }]*/
// Background application that sets a GPIO pin high and low as fast as possible
(function () {
    "use strict";

    if (Microsoft.IoT.Lightning.Providers.LightningProvider.isLightningEnabled) {
        Windows.Devices.LowLevelDevicesController.defaultProvider = Microsoft.IoT.Lightning.Providers.LightningProvider.getAggregateProvider();
    } else {
        console.log("Lightning NOT enabled!");
    }

    var gpioController = Windows.Devices.Gpio.GpioController.getDefault();
    if (gpioController == null) {
        console.log("No GPIO controller available");
        return;
    }

    var pin = gpioController.openPin(5);
    pin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.output);

    while (true) {
        pin.write(Windows.Devices.Gpio.GpioPinValue.high);
        pin.write(Windows.Devices.Gpio.GpioPinValue.low);
    }
})();
