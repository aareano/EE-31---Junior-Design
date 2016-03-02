
// *************************** //
//      --------------------
//            |            |
//             / GATE1       / GATE3
// The        |            |
// H-Bridge   --------------
// schematic  |            |
//             / GATE2       / GATE4
//            |            |
//      --------------------
// *************************** //

// Motion control
// motor speed is a euphemisms for duty cycle, the sign indicates the spin direction
// both speeds @ 100 >> foward at full speed
int rightMotorSpeed = 0;  // varies from -100 to 100
int leftMotorSpeed = 0;   // varies from -100 to 100

// pins
int GATE1 = 3;
int GATE2 = 5;
int GATE3 = 6;
int GATE4 = 9;

enum MotorName { LEFT, RIGHT };

// optic sensor
int RED_LED_1 = 52;
int RED_LED_2 = 53;
int BLUE_LED = 22;

int LED_SENSOR_LEVEL = 7;

// Maximum voltage readings for that color (with the appropriate LED lit)
float BLUE_THRESHOLD = 3.8; //3.3; // commented thresholds work for when sensor is flat on the ground
float RED_THRESHOLD = 4.6; //4.4;  // current thresholds work for when sensor is sliiiightly above the ground

// represents the color the bot has most recently detected
enum Color { BLACK, BLUE, RED };
Color CurrentColor;

// Hall effect sensor
int H_SENSOR = A0;
int H_LED = 32;

int H_THRESHOLD = 0.5; // the voltage needs to drop below this level for the mine to be registered

enum Mine { NONE, FOUND };
Mine MineState;

// Collision Detection
bool CD_enabled = true;
const int BumperCount = 5;
const int DoubleBumperCount = 3;

int BumperDebounceDelay = 50; // the signal needs to be steady for 50ms before it's considered valid

enum BumperState { UP, DOWN, UP_SERVICING, DOWN_SERVICING };
enum DoubleBumperState { SERVICING, SERVICED };
typedef void (*ServiceFunc)();

typedef struct Bumper {
  int pin;
  boolean pinState;   // true => high (depressed), false => low (not depressed)
  int ledPin;
  char name[3];
  BumperState state;
  long timeTriggered;
  ServiceFunc service;
  int serviceTime;
  long lastDebounceTime;
} Bumper;

typedef struct DoubleBumper {
  char name[6];
  DoubleBumperState state;
  long timeTriggered;
  ServiceFunc service;
  int serviceTime;
  Bumper *children[2];
} DoubleBumper;

void service_FL();
void service_FC();
void service_FR();
void service_BL();
void service_BR();
void service_FL_FC();
void service_FC_FR();
void service_BL_BR();

Bumper FL = { .pin = 2,  .pinState = false, .ledPin = 40, "FL", .state = UP, .timeTriggered = 0, .service = service_FL, .serviceTime = 500, .lastDebounceTime = 0 };
Bumper FC = { .pin = 18, .pinState = false, .ledPin = 41, "FC", .state = UP, .timeTriggered = 0, .service = service_FC, .serviceTime = 500, .lastDebounceTime = 0 };
Bumper FR = { .pin = 19, .pinState = false, .ledPin = 42, "FR", .state = UP, .timeTriggered = 0, .service = service_FR, .serviceTime = 500, .lastDebounceTime = 0 };
Bumper BL = { .pin = 20, .pinState = false, .ledPin = 43, "BL", .state = UP, .timeTriggered = 0, .service = service_BL, .serviceTime = 500, .lastDebounceTime = 0 };
Bumper BR = { .pin = 21, .pinState = false, .ledPin = 44, "BR", .state = UP, .timeTriggered = 0, .service = service_BR, .serviceTime = 500, .lastDebounceTime = 0 };

DoubleBumper FL_FC = { "FL_FC", .state = SERVICED, .timeTriggered = 0, .service = service_FL_FC, .serviceTime = 500, .children = { &FL, &FC } };
DoubleBumper FC_FR = { "FC_FR", .state = SERVICED, .timeTriggered = 0, .service = service_FC_FR, .serviceTime = 500, .children = { &FC, &FR } };
DoubleBumper BL_BR = { "BL_BR", .state = SERVICED, .timeTriggered = 0, .service = service_BL_BR, .serviceTime = 500, .children = { &BL, &BR } };

Bumper Bumpers[] = { FL, FC, FR, BL, BR };
DoubleBumper DoubleBumpers[] = { FL_FC, FC_FR, BL_BR };

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);

  pinMode(GATE1, OUTPUT);
  pinMode(GATE2, OUTPUT);
  pinMode(GATE3, OUTPUT);
  pinMode(GATE4, OUTPUT);

  pinMode(RED_LED_1, OUTPUT);
  pinMode(RED_LED_2, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  pinMode(LED_SENSOR_LEVEL, INPUT);

  pinMode(H_SENSOR, INPUT);
  pinMode(H_LED, OUTPUT);

  pinMode(FL.pin, OUTPUT);
  pinMode(FC.pin, OUTPUT);
  pinMode(FR.pin, OUTPUT);
  pinMode(BL.pin, OUTPUT);
  pinMode(BR.pin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FL.pin), FL_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FC.pin), FC_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR.pin), FR_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BL.pin), BL_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BR.pin), BR_bumper_event, CHANGE);

  pinMode(FL.ledPin, OUTPUT);
  pinMode(FC.ledPin, OUTPUT);
  pinMode(FR.ledPin, OUTPUT);
  pinMode(BL.ledPin, OUTPUT);
  pinMode(BR.ledPin, OUTPUT);

  // set initial state
  CurrentColor = BLACK;
  MineState = NONE;
  halt();
}

// the loop routine runs over and over again forever:
// the loop is for changing the state if necessary, then executing the current state.
void loop() {

  // ** UPDATE THE CURRENT STATE (if necessary) ** //
  
  // check for collision
  pull_bumpers();

  // handle collision
  service_collisions();

  // check hall effect sensor
  pull_h_sensor();
  
  // check sound reciever(s)
    
  // check color sensor
  CurrentColor = detectColor();
  
//  Serial.print("CurrentColor: ");
//  Serial.println(CurrentColor);
  
//  switch (CurrentColor) {
//    case BLUE:
//      forward();
//      break;
//    case RED:
//      reverse();
//      break;
//    case BLACK:
//      turnRightInPlace();
//      break;
//  }


  // ** EXECUTE THE CURRENT STATE ** //
  
  // execute mine state
  service_mine();

  // execute driving
  drive();  
  
  // execute (sound) communication
  
  // execute any other state-dependent actions (e.g. light LEDs)
    
  
  // ** EXECUTE STATE-INDEPENDENT ACTIONS (I can't think of any) ** //

  delay(500);
  Serial.println("------------------------");
}

// ******************* COLLISION CONTROL ******************* //

// https://www.arduino.cc/en/Tutorial/Debounce
void pull_bumpers() {
  for(int i = 0; i < BumperCount; i++) {
    // if the input has settled and existed for long enough as the same value, use it!
    if ((millis() - Bumpers[i].lastDebounceTime) > BumperDebounceDelay) {
      Bumpers[i].pinState = digitalRead(Bumpers[i].pin) == HIGH ? true : false;
      
//      Serial.print(Bumpers[i].name);
//      Serial.print(" >> ");
//      Serial.println((int) Bumpers[i].pinState);
    }
  }
}

void service_collisions() {
  
  // update bumper states based on switch levels (pin states)
  for(int i = 0; i < BumperCount; i++) {     
    if (Bumpers[i].pinState) {  // bumper is depressed
      Serial.print(Bumpers[i].name);
      Serial.println(" is pressed");
      
      // bumper.state == DOWN           - should never happen here
      // bumper.state == DOWN_SERVICING - do nothing
      if (Bumpers[i].state == UP || Bumpers[i].state == UP_SERVICING) {
        Serial.print(Bumpers[i].name);
        Serial.println(" >> DOWN");
      
        Bumpers[i].state = DOWN;              // you hit a wall after not being on a wall.
      }
    } else {                // bumper is not depressed
//      Serial.print(Bumpers[i].name);
//      Serial.println(" is NOT pressed");

      // bumper.state == UP           - do nothing
      // bumper.state == DOWN         - should never happen here
      // bumper.state == UP_SERVICING - do nothing
      if (Bumpers[i].state == DOWN_SERVICING) {
        Serial.print(Bumpers[i].name);
        Serial.println(" >> UP_SERVICING");
        Bumpers[i].state == UP_SERVICING;     // we've moved off of the wall
      }
    }

//    Serial.print("Updated state of ");
//    Serial.print(Bumpers[i].name);
//    Serial.print(" :: ");
//    Serial.println(Bumpers[i].state);
  }

  // if two adjacent bumpers are down, service the collision with a double bumper
  if ((FL.state == DOWN || FL.state == DOWN_SERVICING) && (FC.state == DOWN || FC.state == DOWN_SERVICING)) {
    FL_FC.state = SERVICING;
//    Serial.print(FL_FC.name);
//    Serial.println(" >> SERVICING");
  }
  if ((FC.state == DOWN || FC.state == DOWN_SERVICING) && (FR.state == DOWN || FR.state == DOWN_SERVICING)) {
    FC_FR.state = SERVICING;
//    Serial.print(FC_FR.name);
//    Serial.println(" >> SERVICING");
  }
  if ((BL.state == DOWN || BL.state == DOWN_SERVICING) && (BR.state == DOWN || BR.state == DOWN_SERVICING)) {
    BL_BR.state = SERVICING;
//    Serial.print(BL_BR.name);
//    Serial.println(" >> SERVICING");
  }

  // service single bumpers
  for (int i = 0; i < BumperCount; i++) {
    // if the double bumper that encompansses this single bumper is SERVICING, then don't service the single bumper
    if (((FL_FC.children[0] == &Bumpers[i] || FL_FC.children[1] == &Bumpers[i]) && FL_FC.state == SERVICING)
      || ((FC_FR.children[0] == &Bumpers[i] || FC_FR.children[1] == &Bumpers[i]) && FC_FR.state == SERVICING)
      || ((BL_BR.children[0] == &Bumpers[i] || BL_BR.children[1] == &Bumpers[i]) && BL_BR.state == SERVICING)) {
      continue;
    }

    switch (Bumpers[i].state) {
      case UP:
        // nothing to service, do nothing
        break;
      case DOWN:  // we just had impact. start servicing.
        Bumpers[i].state = DOWN_SERVICING;
        Bumpers[i].timeTriggered = millis();
        Bumpers[i].service();
        break;
      case UP_SERVICING:    // cool, we're servicing. keep servicing.
        Bumpers[i].service();   
        break;
      case DOWN_SERVICING:  // cool, we're servicing. keep servicing.
        Bumpers[i].service();
        break;
    }
  }

  // service double bumpers
  for (int i = 0; i < DoubleBumperCount; i++) {
    switch (DoubleBumpers[i].state) {
      case SERVICING:
        DoubleBumpers[i].service();     // service!
        break;
      case SERVICED:
        // we're not serving now, so do nothing
        break;
    }
  }
}

// interrupt to update the time of the last voltage change on a switch
void FL_bumper_event() {
  Serial.println("FL bounced");
  FL.lastDebounceTime = millis();
}

void FC_bumper_event() {
  FC.lastDebounceTime = millis();
}

void FR_bumper_event() {
  FR.lastDebounceTime = millis();
}

void BL_bumper_event() {
  BL.lastDebounceTime = millis();
}

void BR_bumper_event() {
  BR.lastDebounceTime = millis();
}

// function to service the FL bumper when it is triggered individually
void service_FL() {
  Serial.println("Servcing FL");
  if (millis() < FL.timeTriggered + FL.serviceTime) { // service here
    digitalWrite(FL.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FL.ledPin, LOW);
    FL.state = UP;
  }
}


void service_FC() {
  if (millis() < FC.timeTriggered + FC.serviceTime) { // service here
    digitalWrite(FC.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FC.ledPin, LOW);
    FC.state = UP;
  }
}


void service_FR() {
  if (millis() < FR.timeTriggered + FR.serviceTime) { // service here
    digitalWrite(FR.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FR.ledPin, LOW);
    FR.state = UP;
  }
}


void service_BL() {
  if (millis() < BL.timeTriggered + BL.serviceTime) { // service here
    digitalWrite(BL.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(BL.ledPin, LOW);
    BL.state = UP;
  }
}


void service_BR() {
  if (millis() < BR.timeTriggered + BR.serviceTime) { // service here
    digitalWrite(BR.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(BR.ledPin, LOW);
    BR.state = UP;
  }
}

// double bumpers

void service_FL_FC() {
  if (millis() < FR.timeTriggered + FR.serviceTime) { // service here
    digitalWrite(FR.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(FR.ledPin, LOW);
    FL_FC.state = SERVICED;
    FL.state = UP;
    FC.state = UP;
  }
}


void service_FC_FR() {
  if (millis() < BL.timeTriggered + BL.serviceTime) { // service here
    digitalWrite(BL.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(BL.ledPin, LOW);
    FC_FR.state = SERVICED;
    FC.state = UP;
    FR.state = UP;
  }
}


void service_BL_BR() {
  if (millis() < BR.timeTriggered + BR.serviceTime) { // service here
    digitalWrite(BR.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(BR.ledPin, LOW);
    BL_BR.state = SERVICED;
    BL.state = UP;
    BR.state = UP;
  }
}

// ******************* HALL EFFECT SENSOR CONTROL ******************* //

void pull_h_sensor() {
  float reading = calcVolts(analogRead(H_SENSOR));
//  Serial.print("pulling sensor: ");
//  Serial.println(reading);
  if (reading < H_THRESHOLD) {
//    Serial.println("found mine");
    MineState = FOUND;
  }
}

void service_mine() {
  if (MineState == FOUND) {
    digitalWrite(H_LED, HIGH);
  } else {
    digitalWrite(H_LED, LOW);
  }
}

// ******************* COLOR SENSOR CONTROL ******************* //

Color detectColor() {
  // outer if-statement for optimization
  //   - if it's on blue, it's most likely that it will detect blue again
  //   - likewise for red
  if (CurrentColor == BLUE) {
    if (detectBlue()) {         // look for blue first
      return BLUE;
    } else if (detectRed()) {
      return RED;
    } else {
      return BLACK;
    }
  } else  {
    if (detectRed()) {          // look for red first
      return RED;
    } else if (detectBlue()) {
      return BLUE;
    } else {
      return BLACK;
    }
  }
}

bool detectBlue() {
  // illuminate blue LED
  digitalWrite(BLUE_LED, HIGH);
  delay(1);

  // check the sensor
  int reading = analogRead(LED_SENSOR_LEVEL);
  
  // turn off blue LED
  digitalWrite(BLUE_LED, LOW);

//  Serial.print("blue reading: ");
//  Serial.println(calcVolts(reading));
  
  return calcVolts(reading) < BLUE_THRESHOLD;
}

bool detectRed() {
  // illuminate red LED
  digitalWrite(RED_LED_1, HIGH);
  digitalWrite(RED_LED_2, HIGH);
  delay(1);

  // check the sensor
  int reading = analogRead(LED_SENSOR_LEVEL);
  
  // turn off red LED
  digitalWrite(RED_LED_1, LOW);
  digitalWrite(RED_LED_2, LOW);

//  Serial.print("red reading: ");
//  Serial.println(calcVolts(reading));
  
  return calcVolts(reading) < RED_THRESHOLD;
}

// scales 0-1023 to 0-5
float calcVolts(float analogIn) {
  return ( analogIn / 1023 ) * 5;
}

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

        // ***** MOTION CONVENIENCE FUNCTIONS ***** //
// These functions just set the motor speeds, without calling drive(), nothing happens. 

void forward() {
    rightMotorSpeed = 50;
    leftMotorSpeed = 50;
}

void forward_slow() {
    rightMotorSpeed = 25;
    leftMotorSpeed = 25;
}

void reverse() {
    rightMotorSpeed = -50;
    leftMotorSpeed = -50;
}

void halt() {
  rightMotorSpeed = 0;
  leftMotorSpeed = 0;
}

void turnRight() {
  rightMotorSpeed = 0;
  leftMotorSpeed = 50;
}

void turnRightInPlace() {
  rightMotorSpeed = -50;
  leftMotorSpeed = 50;
}

void turnLeft() {
  rightMotorSpeed = 50;
  leftMotorSpeed = 0;
}

void turnLeftInPlace() {
  rightMotorSpeed = 50;
  leftMotorSpeed = -50;  
}

// add more convenience functions as needed

// ******************* MOTOR CONTROL ******************* //

// turn the specified motor in the direction set by the high/low pins at the specified duty cycle (0-100)
void turnMotor(MotorName mName, int highPin, int lowPin, float dutyCycle) {
  int highPWM = 0;
  int lowPWM = 0;
  
  if (mName == RIGHT) {
    int offset = 0;
    highPWM = calcPWM(roundPWM(dutyCycle + offset, 15, 85));
  } else {
    int offset = 0;
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
  return PWM;
}
