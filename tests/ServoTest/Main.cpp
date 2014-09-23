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
int pos = 0;    // variable to store the servo position 

void setup()
{
    myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
}


void loop()
{
    /* Tested to work on 9/26 at 4:41pm */
    //myservo.write(0);
    //delay(1000);
    //myservo.write(180);
    //delay(1000);

    /*Tested to work on 9/26 at 4:44pm */
    myservo.writeMicroseconds(544);
    delay(1000);
    myservo.writeMicroseconds(2400);
    delay(1000);

    Log("ServoIndex: %d\n", myservo.read());
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