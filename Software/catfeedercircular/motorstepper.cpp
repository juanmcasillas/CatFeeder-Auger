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


// servo code
// Servo code.




void MotorServoClass::begin() {
  this->_servo.attach(this->motorPin);
  this->_servo.detach();
}

void MotorServoClass::Clockwise() {
  // to the right
  this->_turn360(1);
}

void MotorServoClass::Anticlockwise() {
  // to the left
  this->_turn360(1, false);
}

void MotorServoClass::Move(int counter, int steps, MotorServoClass *motor) {
  for (int i=0; i< steps; i++) {
    if (counter == 1) motor->Clockwise();
    else motor->Anticlockwise();
  }
}

// This will be entry point to to the work.
void MotorServoClass::Feed(float revolutions, MotorServoClass *motor) {
  float angle = revolutions * 360;
  
  int direction = (angle >= 0 ? -1 : 1); // the opposite
  motor->_turn(direction * 15); // just a little back
  motor->_turn(angle);
}





void MotorServoClass::_turn360(int turns, bool right) {
  int direction = (right ? 0 : 180);
  _servo.attach(this->motorPin);  // uses a simple trick to "stop" the servo
  _servo.write(direction); // center
  for (int i=0; i< turns; i++) {
    delay(1600); // time to give a full turn.
  }
  _servo.detach();
}

void MotorServoClass::_turn(int angle) {
// angle > 0: turn clockwise
// angle < 0: turn anticlockwise.
  
  // I do the metal spiral in the opposite side, so I have to change this to support "positive values"
  int direction = (angle >= 0 ? 180 : 0);
  int angle_abs = abs(angle);
  const int d_turn = 1600;
  // The things spent 1600 seconds in a full turn (calculated, see above)
  // so:
  // 360 -> 1600
  // angle -> x 
  // x = angle*1600/360

  int d_time = ceil(angle_abs * d_turn / 360.0);
  _servo.attach(this->motorPin);
  _servo.write(direction);
  delay(d_time);
  _servo.detach();
}