///////////////////////////////////////////////////////////////////////////////
/// STEPPER MOTOR Adapter /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef __MOTORSTEPPERCLASS_H__
#define __MOTORSTEPPERCLASS_H__

#include <Arduino.h>

#ifndef DBG_OUTPUT_PORT
#define DBG_OUTPUT_PORT Serial
#endif 

#ifndef RELEASE
#define DEBUGLOG(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

class MotorStepperClass {
    public:
        const int stepsPerRev = 4096;  // number of steps for a full rev
        //int stepsPerRev = 4076;  // number of steps for a full rev
    protected:
        // motor: 28BYJ48
        // driver ULN2003
        //                             ESP8266           ULN2003 COLOR
        const int motorPin1 = D1;   // D1  (GPIO14)      IN1     black 
        const int motorPin2 = D2;   // D2  (GPIO12)      IN2     white 
        const int motorPin3 = D5;   // D5  (GPIO13)      IN3     Blue  
        const int motorPin4 = D6;   // D6  (GPIO15)      IN4     Red 
        
        int motorSpeed =  2;      // speed in milis (was 5 but with 1 phase, 2 good)
        int stepCounter = 0;     // step counter
        

    
        //sequence 1-phase
        //less torque, less consuption
        //const int numSteps = 4;
        //const int stepsLookup[4] = { B1000, B0100, B0010, B0001 };
        
        //sequence 2-phase
        //max torque
        // recommended speed: 2 
        const int numSteps = 4;
        const int stepsLookup[4] = { B1100, B0110, B0011, B1001 };
        
        //sequence half-phase
        //wave. Recommended by the manufacturer. Less torque.
        //const int numSteps = 8;
        // recommended speed = 5
        //const int stepsLookup[8] = { B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001 };
    
    public:
        void begin();
        void Clockwise();
        void Anticlockwise();
        static void Move(int spin, int steps, MotorStepperClass *motor); // if (spin == 1) motor->Clockwise();
        static void MoveWithBack(int spin, int steps, MotorStepperClass *motor); // if (spin == 1) motor->Clockwise(); (do 5 steps back to avoid jamming)
        static void OneRevolution(int spin, int revs, MotorStepperClass *motor);   // if (spin == 1) motor->Clockwise();

        // previous functions will be removed.
        static void Feed(float revolutions, MotorStepperClass *motor);   
    protected:

       void _set_output(int step);
};



#endif