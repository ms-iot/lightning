// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"
#include <Servo.h> 

int _tmain(int argc, _TCHAR* argv[])
{
    return RunArduinoSketch();
}

Servo myservo;  // create servo object to control a servo a maximum of eight servo objects can be created 
Servo myservo2;
int pos = 0;    // variable to store the servo position 
int delayAmount = 2000; // used for debugging and spacing out the data
int pin = 3; // the pin that the Servo is on
int pin2 = 5;

void setup()
{
}


void loop()
{
    myservo.attach(pin);  // attaches the servo on pin to the servo object 
    myservo2.attach(pin2);

    myservo2.write(0);
    Log("ServoIndex: %d\n", myservo2.read());
    Log("ServoIndex in Microseconds: %d\n", myservo2.readMicroseconds());
    delay(delayAmount);
    myservo2.write(180);
    Log("ServoIndex: %d\n", myservo2.read());
    Log("ServoIndex in Microseconds: %d\n", myservo2.readMicroseconds());

    /* Tested to work on 9/26 at 4:41pm */
    myservo.write(-90);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);
    myservo.write(0);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);
    myservo.write(180);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);
    myservo.write(200);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);

    /*Tested to work on 9/26 at 4:44pm */
    myservo.writeMicroseconds(544);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);
    myservo.writeMicroseconds(4000);
    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    delay(delayAmount);

    Log("ServoIndex: %d\n", myservo.read());
    Log("ServoIndex in Microseconds: %d\n", myservo.readMicroseconds());
    if (myservo.attached())
    {
        Log("Servo is attached\n");
        Log("Servo is detaching\n");
        myservo.detach();
        if (!myservo.attached())
        {
            Log("Servo is detached\n");
        }
    }
    else
    {
        Log("Servo is not attached\n");
    }

    //for (pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
    //{                                  // in steps of 1 degree 
    //    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    //    delay(15);                       // waits 15ms for the servo to reach the position 
    //}
    //for (pos = 180; pos >= 1; pos -= 1)     // goes from 180 degrees to 0 degrees 
    //{
    //    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    //    delay(15);                       // waits 15ms for the servo to reach the position 
    //}
}