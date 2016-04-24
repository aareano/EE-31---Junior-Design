
// ******************* MOTION CONTROL ******************* //
// The convenience functions tell the motors what we want, drive() makes the motors do it

// sets motors to drive at their respective speed/direction
void drive() {
  if (rightMotorSpeed > 0) {
    rightMotorClockwise(rightMotorSpeed);
  } else {
    rightMotorCounterClockwise(-rightMotorSpeed);
  }
  
  if (leftMotorSpeed > 0) {
    leftMotorClockwise(leftMotorSpeed);
  } else {
    leftMotorCounterClockwise(-leftMotorSpeed);
  } 
}

long get_drive_time(float inches) {
  float forwardSpeed = 7.75; // in/s    // you made need to adjust this constant based on batteries and PWM speed
  float seconds = inches / forwardSpeed;
  return (long) 1000*seconds; // ms
}

long get_rotate_time(int degrees) {
  int timePerDegree = 4.95; // ms    // you made need to adjust this constant based on batteries and PWM speed
  return timePerDegree * degrees;
}

        // ***** MOTION CONVENIENCE FUNCTIONS ***** //
// These functions just set the motor speeds, without calling drive(), nothing happens. 

void forward() {
    rightMotorSpeed = 25;
    leftMotorSpeed = 25;
}

void reverse() {
    rightMotorSpeed = -25;
    leftMotorSpeed = -25;
}

void reverseRight() {
    rightMotorSpeed = 0;
    leftMotorSpeed = -35;
}

void reverseLeft() {
    rightMotorSpeed = -35;
    leftMotorSpeed = 0;
}

void halt() {
  rightMotorSpeed = 0;
  leftMotorSpeed = 0;
}

void turnRight() {
  rightMotorSpeed = 0;
  leftMotorSpeed = 35;
}

void turnRightInPlace() {
  rightMotorSpeed = -35;
  leftMotorSpeed = 35;
}

void turnLeft() {
  rightMotorSpeed = 35;
  leftMotorSpeed = 0;
}

void turnLeftInPlace() {
  rightMotorSpeed = 35;
  leftMotorSpeed = -35;  
}

// add more convenience functions as needed

// ******************* MOTOR CONTROL ******************* //

// turn the specified motor in the direction set by the high/low pins at the specified duty cycle (0-100)
void turnMotor(MotorName mName, int highPin, int lowPin, float dutyCycle) {
  int highPWM = 0;
  int lowPWM = 0;
  
  if (mName == RIGHT) {
    int offset = dutyCycle == 0 ? 0 : 0;
    highPWM = calcPWM(roundPWM(dutyCycle + offset, 15, 85));
  } else {
    int offset = dutyCycle == 0 ? 0 : 3; // 0 offset either way
    highPWM = calcPWM(roundPWM(dutyCycle + offset, 15, 85));
  }

  analogWrite(lowPin, lowPWM);
  analogWrite(highPin, highPWM);
}

        // ***** RIGHT MOTOR ***** //

// clockwise facing the outside of the wheel
void rightMotorClockwise(float dutyCycle) {
  turnMotor(RIGHT, GATE4, GATE3, dutyCycle);
}

// clockwise facing the outside of the wheel
void rightMotorCounterClockwise(float dutyCycle) {
  turnMotor(RIGHT, GATE3, GATE4, dutyCycle);
}

        // ***** LEFT MOTOR ***** //

// clockwise facing the outside of the wheel
void leftMotorClockwise(float dutyCycle) {
  turnMotor(LEFT, GATE1, GATE2, dutyCycle);
}

// clockwise facing the outside of the wheel
void leftMotorCounterClockwise(float dutyCycle) {
  turnMotor(LEFT, GATE2, GATE1, dutyCycle);
}

        // ***** MOTOR CONVENIENCE FUNCTIONS ***** //

// scales 0-100 to 0-255
// duty cycle is measured 0-100, the PWM ouput can range from 0-255
float calcPWM(float dutyCycle) {
  return ( dutyCycle / 100 ) * 255;
}

// acceptable PWM values: {x | x = 0, x = 100, localMin <= x <= localMax}
// basically makes sure PWM is between (inclusive) localMin and localMax, or 0 or 100
int roundPWM(int PWM, int localMin, int localMax) {
  if (localMin <= PWM && PWM <= localMax) {
    // do nothing
  } else if (0 < PWM && PWM < localMin) {     // set local minimum
    PWM = localMin;
  } else if (localMax < PWM && PWM < 100) {   // set local maxiumum
    PWM = localMax;
  }
  PWM = PWM > 100 ? 100 : PWM;    // sanity check
  PWM = PWM < 0 ? 0 : PWM;
  return PWM;
}
