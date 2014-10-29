/** \file servo.h
 * Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
 * Licensed under the BSD 2-Clause License.
 * See License.txt in the project root for license information.
 */

#define MIN_PULSE_WIDTH 544
#define MAX_PULSE_WIDTH 2400
#define DEFAULT_PULSE_WIDTH 1500
#define REFRESH_INTERVAL 20000 // 20 ms
#define MAX_SERVOS 12

/// \brief Allows control of servo motors
/// \details Servos have integrated gears and a shaft that can be
/// precisely controlled. Standard servos allow the shaft to be
/// positioned at various angles, usually between 0 and 180 degrees.
/// Continuous rotation servos allow the rotation of the shaft to
/// be set to various speeds. 
/// \note The Servo library supports up to 12 motors.
class Servo
{
    int _attachedPin;
    uint8_t _servoIndex;
    int _min;
    int _max;

public:
    Servo();

    /// \brief Attach the Servo variable to a pin
    /// \param [in] pin The pin number to which the servo is attached
    /// \param [in] [min] The pulse width, in microseconds, corresponding
    /// to the minimum (0-degree) angle on the servo (defaults to 544)
    /// \param [in] [max] The pulse width, in microseconds, corresponding
    /// to the maximum (180-degree) angle on the servo (defaults to 2400)
    /// \returns zero
    /// \see <a href="http://arduino.cc/en/Reference/ServoAttach" target="_blank">origin: Arduino::Servo::attach</a>
    uint8_t attach(int pin, int min, int max);
    uint8_t attach(int pin);

    /// \brief Detach the Servo variable from its pin.
    /// \see <a href="http://arduino.cc/en/Reference/ServoDetach" target="_blank">origin: Arduino::Servo::detach</a>
    void detach();

    /// \brief Writes a value to the servo, controlling the shaft accordingly.
    /// \details On a standard servo, this will set the angle of the shaft (in
    /// degrees), moving the shaft to that orientation. On a continuous rotation
    /// servo, this will set the speed of the servo (with 0 being full-speed in
    /// one direction, 180 being full speed in the other, and a value near 90
    /// being no movement).
    /// \param [in] value The value to write to the servo, from 0 to 180
    /// \see <a href="http://arduino.cc/en/Reference/ServoWrite" target="_blank">origin: Arduino::Servo::write</a>
    void write(int value);

    /// \brief Writes a value in microseconds (uS) to the servo, controlling
    /// the shaft accordingly.
    /// \details On a standard servo, this will set the angle of the shaft.
    /// On standard servos a parameter value of 1000 is fully counter-clockwise,
    /// 2000 is fully clockwise, and 1500 is in the middle.
    /// \param [in] value The value of the parameter in microseconds
    /// \note Continuous-rotation servos will respond to the writeMicrosecond
    /// function in an analogous manner to the write function.
    /// \note Some manufactures do not follow this standard very closely so that
    /// servos often respond to values between 700 and 2300. Feel free to
    /// increase these endpoints until the servo no longer continues to increase
    /// its range.
    /// \warning Attempting to drive a servo past its endpoints (often indicated
    /// by a growling sound) is a high-current state, and should be avoided.
    /// \see <a href="http://arduino.cc/en/Reference/ServoWriteMicroseconds" target="_blank">origin: Arduino::Servo::writeMicroseconds</a>
    void writeMicroseconds(int value);

    /// \brief Read the current angle of the servo
    /// \details The value passed to the last call to write().
    /// \return The angle of the servo, from 0 to 180 degrees
    /// \see <a href="http://arduino.cc/en/Reference/ServoRead" target="_blank">origin: Arduino::Servo::read</a>
    int read();

    /// \brief Read the current angle of the servo (in microseconds)
    /// \details The value passed to the last call to writeMicroseconds().
    /// \return The angle of the servo, from 1000 to 2000 degrees (approx.)
    int readMicroseconds();

    /// \brief Check whether the Servo variable is attached to a pin.
    /// \return True if the servo is attached to pin; false otherwise.
    /// \see <a href="http://arduino.cc/en/Reference/ServoAttached" target="_blank">origin: Arduino::Servo::attached</a>
    bool attached();
};
