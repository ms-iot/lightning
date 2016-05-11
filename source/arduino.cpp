#include "pch.h"

#include "arduino.h"

// 
// Global extern exports
//
AdcClass g_adc;
ULONG g_analogValueBits = 10;
ULONG g_pwmResolutionBits = 8;

char* dtostrf(double value, char width, uint8_t precision, char* buffer)
{
    // dtostrf is defined that the buffer is width plus null
    sprintf_s(buffer, width + 1, "%*.*f", width, precision, value);

    return buffer;
}

void tone(int pin, unsigned int frequency)
{
    UNREFERENCED_PARAMETER(pin);
    UNREFERENCED_PARAMETER(frequency);
}

void tone(int pin, unsigned int frequency, unsigned long duration)
{
    UNREFERENCED_PARAMETER(pin);
    UNREFERENCED_PARAMETER(frequency);
    UNREFERENCED_PARAMETER(duration);
}

void noTone(int pin)
{
    UNREFERENCED_PARAMETER(pin);
}

//
// Pauses the program for the amount of time (in microseconds) 
// specified as parameter.
//
void delayMicroseconds(unsigned int us)
{
    LARGE_INTEGER us64;
    us64.QuadPart = us;
    _WindowsTime.delayMicroseconds(us64);
}

//
// Pauses the program for the amount of time (in miliseconds) 
// specified as parameter.
//
void delay(unsigned long ms)
{
    _WindowsTime.delay(ms);
}

// Returns the number of milliseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 50 days.
unsigned long millis(void)
{
    return _WindowsTime.millis();
}

// Returns the number of microseconds since the currently running program started. 
// This number will overflow (go back to zero), after approximately 70 minutes.
unsigned long micros(void)
{
    return _WindowsTime.micros();
}

//
// Set the digital pin (IO0 - IO13) to the specified state.
// If the analog pins (A0-A5) are configured as digital IOs,
// also sets the state of these pins.
// A0-A5 are mapped to 14-19
// 
// Examples:
//
//  // set IO4 high.
//  digitalWrite(4, 1);
//  
//  // set A1 low
//  digitalWrite(15, 0);
//
void digitalWrite(unsigned int pin, unsigned int state)
{
    HRESULT hr;

    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, hr);
    }

    if (state != LOW)
    {
        // Emulate Arduino behavior here. Code like firmata uses bitmasks to set
        // ports, and will pass something like value & 0x20 and expect that to be high.
        state = HIGH;
    }

    hr = g_pins.setPinState(pin, state);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred setting pin: %d to state: %d, Error: %08x", pin, state, hr);
    }
}

//
// Reads the value from the digital pin (IO0 - IO13).
// A0-A5 are mapped to 14-19
//
// Return Value:
//
// 1 for HIGH, 0 for LOW or error
// 
// Example:
//
//  // read IO4.
//  int val = digitalRead(4);
//
int digitalRead(int pin)
{
    HRESULT hr;
    ULONG readData = 0;

    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);
    if (SUCCEEDED(hr))
    {
        hr = g_pins.getPinState(pin, readData);
    }
    if (FAILED(hr))
    {
        // On error return LOW per docs.
        readData = LOW;
    }

    return readData;
}

/// Perform an analog to digital conversion on one of the analog inputs.
/**
\param[in] pin The analog pin to read (A0-A5, or 0-5).
\return Digitized analog value read from the pin.
\note The number of bits of the digitized analog value can be set by calling the
analogReadResolution() API.  By default ten bits are returned (0-1023 for 0-5v pin voltage).
\sa analogReadResolution
*/
int analogRead(int pin)
{
    HRESULT hr;
    ULONG value;
    ULONG bits;
    ULONG ioPin = pin;
    BoardPinsClass::BOARD_TYPE board;

    hr = g_pins.getBoardType(board);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error getting board type.  Error: 0x%08x", hr);
    }

    switch (board)
    {
    case BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE:
        // Translate the pin number passed in to an Arduino GPIO Pin number.
        if ((pin >= 0) && (pin < NUM_ANALOG_PINS))
        {
            ioPin = A0 + pin;
        }

        // Make sure the pin is configured as an analog input.
        hr = g_pins.verifyPinFunction(ioPin, FUNC_AIN, BoardPinsClass::NO_LOCK_CHANGE);

        // If we failed to set the pin as an analog input and it is in the range of board pins.
        if (FAILED(hr))
        {
            ThrowError(hr, "Error occurred verifying pin: %d function: ANALOG_IN, Error: 0x%08x", ioPin, hr);
        }
        break;

    case BoardPinsClass::BOARD_TYPE::MBM_BARE:
    case BoardPinsClass::BOARD_TYPE::PI2_BARE:
        // Translate the pin number to a fake pin number.
        if (pin < A0)
        {
            ioPin = A0 + pin;
        }
        break;

    default:
        ThrowError(hr, "Unrecognized board type: 0x%08x", board);
    }

    // Perform the read.
    hr = g_adc.readValue(ioPin, value, bits);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error performing analogRead on pin: %d, Error: 0x%08x", pin, hr);
    }

    // Scale the digitized analog value to the currently set analog read resolution.
    if (g_analogValueBits > bits)
    {
        value = value << (g_analogValueBits - bits);
    }
    else if (bits > g_analogValueBits)
    {
        value = value >> (bits - g_analogValueBits);
    }

    return value;
}

/// Set the number of bits returned by an analogRead() call.
/**
\param[in] bits The number of bits returned from an analogRead() call.
\note If more bits are specified than are natively produced by the ADC on the board
the digitized analog values are padded with zeros.  If fewer bits are specified, analog
values truncated to the desired length.
*/
void analogReadResolution(int bits)
{
    if ((bits < 1) || (bits > 32))
    {
        ThrowError(E_INVALIDARG, "Attempt to set analog read resolution to %d bits.  Supported range: 1-32.", bits);
    }
    g_analogValueBits = bits;
}

/// Set the reference voltage used for analog inputs.
/**
The Arduino only supports an internal 5v reference.  Attempting to select any other
reference than DEFAULT throws an error.
\param[in] type The type of analong reference desired.
\note DEFAULT - ok, INTERNAL, INTERNAL1V1, INTERNAL2V56 or EXTERNAL - error.
*/
void analogReference(int type)
{
    if (type != DEFAULT)
    {
        ThrowError(E_INVALIDARG, "The only supported analog reference is DEFAULT.");
    }
}


/// Set the PWM duty cycle for a pin.
/**
\param[in] pin The number of the pin for the PWM output.  On boards with built-in PWM support
this is a GPIO pin, on boards that use an external PWM chip, this is a pseudo pin number named
PWM0-PWMn, where "n" is one less than the number of PWM pins.
\param[in] dutyCycle The high pulse time, range 0 to pwm_resolution_count - 1, (defaults
to a count of 255, for 8-bit PWM resolution.)
\Note: This call throws an error if the pin number is outside the range supported
on the board, or if a pin that does not support PWM is specified.
*/
void analogWrite(unsigned int pin, unsigned int dutyCycle)
{
    HRESULT hr;
    ULONG ioPin = pin;
    BoardPinsClass::BOARD_TYPE board;
    ULONGLONG scaledDutyCycle;

    hr = g_pins.getBoardType(board);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error getting board type.  Error: 0x%08x", hr);
    }

    switch (board)
    {
    case BoardPinsClass::BOARD_TYPE::MBM_IKA_LURE:

        // Verify the pin is in PWM mode, and configure it for PWM use if not.
        hr = g_pins.verifyPinFunction(ioPin, FUNC_PWM, BoardPinsClass::NO_LOCK_CHANGE);

        if (FAILED(hr))
        {
            ThrowError(hr, "Error occurred verifying pin: %d function: PWM, Error: %08x", ioPin, hr);
        }
        break;

    case BoardPinsClass::BOARD_TYPE::MBM_BARE:
    case BoardPinsClass::BOARD_TYPE::PI2_BARE:
        // Translate the PWM channel numbers to fake pin numbers.
        if (pin < PWM0)
        {
            ioPin = PWM0 + pin;
        }
        break;

    default:
        ThrowError(E_INVALIDARG, "Unrecognized board type: 0x%08x", board);
    }

    // Scale the duty cycle passed in using the current analog write resolution.
    if ((g_pwmResolutionBits < 32) && (dutyCycle >= (1UL << g_pwmResolutionBits)))
    {
        ThrowError(E_INVALIDARG, "Specified duty cycle: %d is greater than PWM resolution: %d bits.", dutyCycle, g_pwmResolutionBits);
    }
    scaledDutyCycle = (((ULONGLONG)dutyCycle * (1ULL << 32)) + (1ULL << (g_pwmResolutionBits - 1))) / (1ULL << g_pwmResolutionBits);

    // Set the PWM duty cycle.
    hr = g_pins.setPwmDutyCycle(ioPin, (ULONG)scaledDutyCycle);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred setting pin: %d PWM duty cycle to: %d, Error: %08x", ioPin, dutyCycle, hr);
    }
}

/// Set the number of bits used to specify PWM duty cycles to analogWrite().
/**
\param[in] bits The number of bits to use for analogWrite() duty cycle values.
*/
void analogWriteResolution(int bits)
{
    if ((bits < 1) || (bits > 32))
    {
        ThrowError(E_INVALIDARG, "Attempt to set analog write resolution to %d bits.  Supported range: 1-32.", bits);
    }
    g_pwmResolutionBits = bits;
}

/// Configure a pin for input or output duty.
/**
\param[in] pin The number of the pin (D0-D13, A0, A5)
\param[in] mode The desired pin mode (INPUT, OUTPUT, INPUT_PULLUP)
*/
void pinMode(unsigned int pin, unsigned int mode)
{
    HRESULT hr;

    // Make sure this pin is not already locked for a conflicting use.
    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, hr);
    }

    switch (mode)
    {
    case INPUT:
        hr = g_pins.setPinMode(pin, DIRECTION_IN, false);

        if (FAILED(hr))
        {
            ThrowError(hr, "Error setting mode: INPUT for pin: %d, Error: 0x%08x", pin, hr);
        }
        break;
    case OUTPUT:
        hr = g_pins.setPinMode(pin, DIRECTION_OUT, false);

        if (FAILED(hr))
        {
            ThrowError(hr, "Error setting mode: OUTPUT for pin: %d, Error: 0x%08x", pin, hr);
        }
        break;
    case INPUT_PULLUP:
        hr = g_pins.setPinMode(pin, DIRECTION_IN, true);

        if (FAILED(hr))
        {
            ThrowError(hr, "Error setting mode: INPUT_PULLUP for pin: %d, Error: 0x%08x", pin, hr);
        }
        break;
    default:
        ThrowError(E_INVALIDARG, "Invalid mode: %d specified for pin: %d.", mode, pin);
    }
}

uint8_t shiftIn(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_)
{
    uint8_t buffer(0);

    for (uint8_t loop_count = 0, bit_index = 0; loop_count < 8; ++loop_count) {
        if (bit_order_ == LSBFIRST) {
            bit_index = loop_count;
        }
        else {
            bit_index = (7 - loop_count);
        }

        digitalWrite(clock_pin_, HIGH);
        buffer |= (digitalRead(data_pin_) << bit_index);
        digitalWrite(clock_pin_, LOW);
    }

    return buffer;
}

void shiftOut(uint8_t data_pin_, uint8_t clock_pin_, uint8_t bit_order_, uint8_t byte_)
{
    for (uint8_t loop_count = 0, bit_mask = 0; loop_count < 8; ++loop_count) {
        if (bit_order_ == LSBFIRST) {
            bit_mask = (1 << loop_count);
        }
        else {
            bit_mask = (1 << (7 - loop_count));
        }

        digitalWrite(data_pin_, (byte_ & bit_mask));
        digitalWrite(clock_pin_, HIGH);
        digitalWrite(clock_pin_, LOW);
    }

    return;
}

/// Attach a callback routine to a GPIO interrupt.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
*/
void attachInterrupt(uint8_t pin, std::function<void(void)> func, int mode)
{
    HRESULT hr;

    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, hr);
    }

    hr = g_pins.attachInterrupt(pin, func, mode);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred attaching interrupt to pin: %d", pin);
    }
}

/// Attach a callback routine to a GPIO interrupt, with return of interrupt information.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
*/
void attachInterruptEx(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER)> func, int mode)
{
    HRESULT hr;

    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, hr);
    }

    hr = g_pins.attachInterruptEx(pin, func, mode);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred attaching interrupt to pin: %d", pin);
    }
}

/// Attach a callback routine to a GPIO interrupt, with return of interrupt information.
/**
\param[in] pin The number of the board pin for which interrupts are wanted.
\param[in] fund The function to be called when an interrupt occurs for the specified pin.
\param[in] mode The type of pin state changes that should cause interrupts.
\param[in] context An optional parameter to pass to the callback function.
*/
void attachInterruptContext(uint8_t pin, std::function<void(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER, PVOID)> func, void* context, int mode)
{
    HRESULT hr;

    hr = g_pins.verifyPinFunction(pin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred verifying pin: %d function: DIGITAL_IO, Error: %08x", pin, hr);
    }

    hr = g_pins.attachInterruptContext(pin, func, context, mode);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred attaching interrupt to pin: %d", pin);
    }
}

/// Indicate GPIO interrupt callbacks are no longer wanted for a pin.
/**
\param[in] pin The number of the board pin for which interrupts are to be detached.
*/
void detachInterrupt(uint8_t pin)
{
    HRESULT hr;

    hr = g_pins.detachInterrupt(pin);
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred detaching interrupt for pin: %d", pin);
    }
}

/// Turn back on interrupt callbacks that have previously been disabled.
void interrupts()
{
    HRESULT hr;

    hr = g_pins.enableInterrupts();
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred enabling interrupts.");
    }
}

/// Temporarily disable delivery of all interrupt callbacks.
void noInterrupts()
{
    HRESULT hr;

    hr = g_pins.disableInterrupts();
    if (FAILED(hr))
    {
        ThrowError(hr, "Error occurred disabling interrupts.");
    }
}

//
// Initialize pseudo random number generator with seed
//
void randomSeed(unsigned int seed)
{
    if (seed != 0) {
        _WindowsRandom.Seed(seed);
    }
}

//
// Generate pseudo random number with upper bound max
//
long random(long max)
{
    if (max == 0) {
        return 0;
    }
    return _WindowsRandom.Next() % max;
}

//
// Generate pseudo random number in the range min - max
//
long random(long min, long max)
{
    if (min >= max) {
        return min;
    }
    long diff = max - min;
    return random(diff) + min;
}
