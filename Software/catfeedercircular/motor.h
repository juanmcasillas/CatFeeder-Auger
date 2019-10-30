///////////////////////////////////////////////////////////////////////////////
/// STEPPER MOTOR Adapter /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef __MOTORSTEPPERCLASS_H__
#define __MOTORSTEPPERCLASS_H__

#include <Arduino.h>
#include <Servo.h>

#ifndef DBG_OUTPUT_PORT
#define DBG_OUTPUT_PORT Serial
#endif 

#ifndef RELEASE
#define DEBUGLOG(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

// power:  Red (5V) -> 5V ARDUINO PIN
// Black/Brown: Ground -> GROUND ARDUINO PIN
// Yellow/Orange/White: Control -> D12
// 23-01-2019 Updated to 360 (see https://www.instructables.com/id/Convert-Towerpro-MG995-servo-to-continuous-rotatio/)

class MotorServoClass {
    public:

    protected:
        const int motorPin = D4;
        Servo _servo; 
    public:
        void begin();
        void Clockwise();
        void Anticlockwise();
        static void Move(int counter, int steps, MotorServoClass *motor);
        static void Feed(float revolutions, MotorServoClass *motor);
        void _turn360(int turns=1, bool right=true);
        void _turn(int angle);
};

#endif