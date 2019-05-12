///////////////////////////////////////////////////////////////////////////////
/// STEPPER MOTOR Adapter /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "motorstepper.h"

void MotorStepperClass::begin() {
    pinMode(this->motorPin1, OUTPUT);
    pinMode(this->motorPin2, OUTPUT);
    pinMode(this->motorPin3, OUTPUT);
    pinMode(this->motorPin4, OUTPUT);
}

void MotorStepperClass::Clockwise() {
  this->stepCounter++;
  if (this->stepCounter >= this->numSteps) this->stepCounter = 0;
  this->_set_output(this->stepCounter);
  delay(this->motorSpeed);


  
}
 
void MotorStepperClass::Anticlockwise() {
  this->stepCounter--;
  if (this->stepCounter < 0) this->stepCounter = this->numSteps - 1;
  this->_set_output(this->stepCounter);
  delay(this->motorSpeed);


}
 
void MotorStepperClass::_set_output(int step) {
  digitalWrite(this->motorPin1, bitRead(this->stepsLookup[step], 0));
  digitalWrite(this->motorPin2, bitRead(this->stepsLookup[step], 1));
  digitalWrite(this->motorPin3, bitRead(this->stepsLookup[step], 2));
  digitalWrite(this->motorPin4, bitRead(this->stepsLookup[step], 3));
}

// static implementation overloading stepCounter.
void MotorStepperClass::Move(int spin, int steps, MotorStepperClass *motor) {
  for (int i=0; i< steps; i++) {
    if (spin == 1) motor->Clockwise();
    else motor->Anticlockwise();
  }
}

void MotorStepperClass::MoveWithBack(int spin, int steps, MotorStepperClass *motor) {
  
  // JMC TODO
  
  int gap = 10 + random(10,20+1);

  for (int i=0; i< steps; i++) {

    // move each % steps back, to avoid jamming
    if (i % gap == 0) {

      int back = random(60, 100+1);

      for (int j=0; j< back; j++) {
        if (spin == 1) motor->Anticlockwise();
        else motor->Clockwise();  
      }
      for (int j=0; j< back; j++) {
        if (spin == 1) motor->Clockwise();
        else motor->Anticlockwise();  
      }      
    }

    if (spin == 1) motor->Clockwise();
    else motor->Anticlockwise();
  }
}

// issue a full revolution using the API
void MotorStepperClass::OneRevolution(int spin, int revs, MotorStepperClass *motor) {
  int steps = (revs < 2 ? motor->stepsPerRev : (motor->stepsPerRev) * (revs/2)); 
  MotorStepperClass::MoveWithBack(spin, steps, motor);
}

