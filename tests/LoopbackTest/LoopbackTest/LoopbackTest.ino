/// LoopBacktest 
/// Tests using the loopback hardware On Raspberry Pi 2
///


/// GPIO Connections
/// 14 <-> 15
/// 18 <-> 23
/// 24 <-> 25
///  7 <-> 12
/// 16 <-> 20
/// 21 <-> 26
/// 19 <-> 13
///  6 <->  5
/// 22 <-> 27
/// 17 <->  4

/// PWM -> ADC
/// 0	->	1
/// 1	->	2
/// 2	->	3 
/// 3	->	4
/// 4	->	5
/// 5	->	6
/// 6	->	7
/// 7	->	8

typedef struct 
{
	byte from;
	byte to;
} TestingPinMap;

TestingPinMap g_testingGPIOPinMap[] = 
{
	{GPIO14, GPIO15},
	{GPIO18, GPIO23},
	{GPIO24, GPIO25},
	{ GPIO7, GPIO12},
	{GPIO16, GPIO20},
	{GPIO21, GPIO26},
	{GPIO19, GPIO13},
	{ GPIO6, GPIO5},
	{GPIO22, GPIO27},
	{GPIO17, GPIO4}
};


TestingPinMap g_testingPWMPinMap[] =
{
	{ PWM0, A0 },
	{PWM1, A1},
	{PWM2, A2},
	{PWM3, A3},
	{PWM4, A4},
	{PWM5, A5},
	{PWM6, A6},
	{PWM7, A7}
};

void setup()
{
	bool failures = false;
    // put your setup code here, to run once:

	for (size_t i = 0; i < _countof(g_testingGPIOPinMap); i++)
	{
		pinMode(g_testingGPIOPinMap[i].from, OUTPUT);
		pinMode(g_testingGPIOPinMap[i].to, OUTPUT);
		
		// Clear first 
		digitalWrite(g_testingGPIOPinMap[i].from, LOW);
		digitalWrite(g_testingGPIOPinMap[i].to, LOW);

		// Set to to input.
		pinMode(g_testingGPIOPinMap[i].to, INPUT);

		// Test High
		digitalWrite(g_testingGPIOPinMap[i].from, HIGH);
		if (digitalRead(g_testingGPIOPinMap[i].to) != HIGH)
		{
			Log("Failed loopback on physical pin combo %d <-> %d\n", g_testingGPIOPinMap[i].from, g_testingGPIOPinMap[i].to);
			failures = true;
		}

		// Then low
		digitalWrite(g_testingGPIOPinMap[i].from, LOW);
		if (digitalRead(g_testingGPIOPinMap[i].to) != LOW)
		{
			Log("Failed loopback on physical pin combo %d <-> %d\n", g_testingGPIOPinMap[i].from, g_testingGPIOPinMap[i].to);
			failures = true;
		}
	}

	for (size_t i = 0; i < _countof(g_testingPWMPinMap); i++)
	{
		analogWrite(g_testingPWMPinMap[i].from, 0);
		int valueLow = analogRead(g_testingPWMPinMap[i].to);
		if (valueLow != 0)
		{
			Log("Failed loopback on analog pin combo %d <-> %d\n", g_testingPWMPinMap[i].from, g_testingPWMPinMap[i].to);
			failures = true;
		}

		analogWrite(g_testingPWMPinMap[i].from, 255);
		int valueHigh = analogRead(g_testingPWMPinMap[i].to);
		if (valueHigh == 0)
		{
			Log("Failed loopback on analog pin combo %d <-> %d\n", g_testingPWMPinMap[i].from, g_testingPWMPinMap[i].to);
			failures = true;
		}
    
        // Prevent inductive coupling.
        analogWrite(g_testingPWMPinMap[i].from, 0);
    }

    Log("***********************************************************************************************************\n");
    Log("***********************************************************************************************************\n");
    if (failures)
	{
		Log("*******                                      Failures Detected                                      *******\n");
	}
    else
    {
        Log("*******                                      All is Well                                            *******\n");
    }
    Log("***********************************************************************************************************\n");
    Log("***********************************************************************************************************\n");
}

void loop()
{
    // put your main code here, to run repeatedly:

    digitalWrite(GPIO5, LOW);
    delay(500);
    digitalWrite(GPIO5, HIGH);
    delay(500);
}
