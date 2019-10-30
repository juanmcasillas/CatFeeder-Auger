///////////////////////////////////////////////////////////////////////////////
/// STEPPER MOTOR Adapter /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "motor.h"

void MotorServoClass::begin() {
  //this->_servo.attach(this->motorPin);
  //this->_servo.detach();
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