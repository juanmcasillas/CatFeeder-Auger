///////////////////////////////////////////////////////////////////////////////
/// STEPPER MOTOR Adapter /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "motorstepper.h"

void MotorStepperClass::begin() {
    pinMode(this->motorPin1, OUTPUT);
    pinMode(this->motorPin2, OUTPUT);
    pinMode(this->motorPin3, OUTPUT);
    pinMode(this->motorPin4, OUTPUT);
    this->stepCounter = 0;
}

void MotorStepperClass::Clockwise() {

  this->_set_output(this->stepCounter);
  
  this->stepCounter++;
  if (this->stepCounter >= this->numSteps) this->stepCounter = 0;

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


// implement some kind of jitter to avoid jamming.

void MotorStepperClass::MoveWithJitter(int spin, int steps, MotorStepperClass *motor) {
  
  int steps_one_revolution = static_cast<int>(ceil( (motor->stepsPerRev/(motor->numSteps/2.0)) * 1.0));
  int steps_per_degree = static_cast<int>(ceil(steps_one_revolution / 360.0)); 

  int MIN_DEGREE = 15;
  int MAX_DEGREE = 30;

  // motivation, go a little back, then walk, then go a little back again.
  // do the jitter. use back_degress to gap.



  int i=0;
  while (i< steps) {

    //if (i % (back_degrees*steps_per_degree) == 0) {
    if (random(0, 100) < 5) { 

      // ok, go back for some angles, then return, and move forward.

      int back_jitter = random(MIN_DEGREE,MAX_DEGREE+1);

      for (int j=0; j< steps_per_degree * back_jitter; j++) {
        if (spin == 1) motor->Anticlockwise();
        else motor->Clockwise();
      }

      // return to the position and advance double

      for (int j=0; j< steps_per_degree * back_jitter; j++) {
        if (spin == 1) motor->Clockwise();
        else motor->Anticlockwise();
      }
    
      // duplicate for to allow i advance
      for (int j=0; j< steps_per_degree * back_jitter; j++) {
        if (spin == 1) motor->Clockwise();
        else motor->Anticlockwise();
        i++;
      }
      

    }
    // advance
    if (spin == 1) motor->Clockwise();
    else motor->Anticlockwise();
    i++;

  }

}

// This will be entry point to to the work.
void MotorStepperClass::Feed(float revolutions, MotorStepperClass *motor) {
  
  int spin = 0; // hardcode to test
  int steps = static_cast<int>(ceil( (motor->stepsPerRev/(motor->numSteps/2.0)) * revolutions));
  
  MotorStepperClass::MoveWithJitter(spin, steps, motor);
  //MotorStepperClass::Move(spin, steps, motor);
}
