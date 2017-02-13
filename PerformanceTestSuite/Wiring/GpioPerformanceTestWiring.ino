// Application that sets a GPIO pin high and low as fast as possible

// Use GPIO pin 5
const unsigned int LED_PIN = GPIO5;

void setup()
{
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
}
