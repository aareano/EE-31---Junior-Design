
// *************************** //
//      --------------------
//            |            |
//             \ GATE2      \ GATE3
// The        | |          | |
// H-Bridge   | |          | |
// schematic  | |          | |
//             / GATE1      / GATE4
//            |            |
//      --------------------
// *************************** //

// Motion control
// motor speed is a euphemisms for duty cycle, the sign indicates the spin direction
// both speeds @ 100 >> foward at full speed
int rightMotorSpeed = 0;  // varies from -100 to 100
int leftMotorSpeed = 0;   // varies from -100 to 100

// pins
int GATE1 = 9;   // orange
int GATE2 = 6;   // yellow
int GATE3 = 10;  // grey
int GATE4 = 11;  // purple

enum MotorName { LEFT, RIGHT };

int time360 = 1475; // ms
int time180 = 750; // ms

// optic sensor
int RED_LED = 50;
int BLUE_LED = 51;

int LED_SENSOR_LEVEL = A1;

// Maximum voltage readings for that color (with the appropriate LED lit)
float BLUE_THRESHOLD = 4.6;
float RED_THRESHOLD = 4.83;

// represents the color the bot has most recently detected
enum Color { BLACK, BLUE, RED };
Color ColorState;

// Line Following
enum Path { PATH_FINDING, ON_PATH, RECOVERING };
enum Side { SEARCHING_LEFT, SEARCHING_RIGHT };
Color PathToFollow;
Side SearchSide;
Path PathState;
long BaseSearchTime = 300;  // this value is later lowered to 100
long SearchTime = 300;      // this is how long to search for the line on the left or right when we get off of it
long MinSearchTime = 50;    // the bot needs to turn at least this much before it starts checking for the path again
long SearchStartTime = 0;   // this will be edited when we start searching for the line when we get off of it

enum TurnDirection {L, R};
bool TurnDirection = L;
Color LAST_STATE = BLUE;

// Hall effect sensor
int H_SENSOR = A0;
int H_LED = 32;

float H_THRESHOLD = 0.5; // the voltage needs to drop below this level for the mine to be registered

enum Mine { NONE, FOUND };
Mine MineState;

// Collision Detection
const int BumperCount = 4;
const int DoubleBumperCount = 2;

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
  long serviceTime;
  long lastDebounceTime;
} Bumper;

typedef struct DoubleBumper {
  char name[6];
  DoubleBumperState state;
  ServiceFunc service;
  long serviceTime;
  Bumper *children[2];
} DoubleBumper;

void service_FL();
void service_FC();
void service_FR();
void service_B();
void service_FL_FC();
void service_FC_FR();

Bumper fl = { .pin = 2,  .pinState = false, .ledPin = 40, "FL", .state = UP, .timeTriggered = 0, .service = service_FL, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper fc = { .pin = 18, .pinState = false, .ledPin = 41, "FC", .state = UP, .timeTriggered = 0, .service = service_FC, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper fr = { .pin = 19, .pinState = false, .ledPin = 42, "FR", .state = UP, .timeTriggered = 0, .service = service_FR, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper b  = { .pin = 20, .pinState = false, .ledPin = 43, "B",  .state = UP, .timeTriggered = 0, .service = service_B,  .serviceTime = 1000, .lastDebounceTime = 0 };

Bumper *FL = &fl;
Bumper *FC = &fc;
Bumper *FR = &fr;
Bumper *B  = &b;

DoubleBumper fl_fc = { "FL_FC", .state = SERVICED, .service = service_FL_FC, .serviceTime = 1000, .children = { FL, FC } };
DoubleBumper fc_fr = { "FC_FR", .state = SERVICED, .service = service_FC_FR, .serviceTime = 1000, .children = { FC, FR } };

DoubleBumper *FL_FC = &fl_fc;
DoubleBumper *FC_FR = &fc_fr;

Bumper *Bumpers[] = { FL, FC, FR, B };
DoubleBumper *DoubleBumpers[] = { FL_FC, FC_FR };

// Communication
int commsIn = A2;
int commsOut = 36;
int commsAlert = 37;
int alertRed = 32;
int alertYellow = 28;
int alertBlue = 30;
enum Message { FOUND_MINE_SCARLET_WITCH, PATH_END, FOUND_MINE_NIGHTWING, DANCE};
enum Comms { COMMS_LISTENING, COMMS_RECEIVING };
Comms CommsState;

// Combination Lock
enum LockInput { LOCK_LEFT, LOCK_CENTER, LOCK_RIGHT, LOCK_EMPTY };
int comboLength = 5;
LockInput correctCombo[] = { LOCK_LEFT, LOCK_CENTER, LOCK_RIGHT, LOCK_LEFT, LOCK_RIGHT };
LockInput userCombo[] = { LOCK_EMPTY, LOCK_EMPTY, LOCK_EMPTY, LOCK_EMPTY, LOCK_EMPTY };
int lockInputNum = 0;

// Master State
enum Master { 
  // challenge 1
  LOCKED,           // LOCKED is challenge 3
  SETTINGS, 
  APPLY_SETTINGS, 
  LISTENING_MY_TURN, 
  FIND_WALL,
  DISCOVER_PATH,  
  FOLLOW_PATH_1, 
  LISTENING_MINE_SCARLET_WITCH,
  LISTENING_MINE_NIGHTWING, 
  FOLLOW_PATH_2, 
  FINAL_COLLISION, 
  FINAL_WAIT, 
  END,
  QUIET,
  
  // challenge 2
  HALT,
  FORWARD_12,
  ROTATE_RIGHT_180,   // arbitrary, could be LEFT
  BACK_3,
  TURN_LEFT,
  TURN_RIGHT_1,
  TURN_RIGHT_2,
  TURN_RIGHT_3,
  
  // test
  TEST_TRANSMITTER,
  TEST_RECEIVER,
  TEST_ROTATION_TIME,
  TEST_COLOR_DETECTION,
  TEST_HALL_EFFECT,
  TEST_PATH_FINDING
};
Master InitializeSequence[] = { LOCKED, SETTINGS, APPLY_SETTINGS };
Master ScarletWitchSequence[] = {
  FIND_WALL,        // find wall (collision)
  DISCOVER_PATH,    // back up from wall until on top of path
  FOLLOW_PATH_1,    // follow path to find mine
  LISTENING_MINE_SCARLET_WITCH,   // wait for 300ms response from command center
  FOLLOW_PATH_2,    // follow path to find end 
  FINAL_COLLISION,  // blinks led
  FINAL_WAIT,       // waits for 300 ms from other bot
  END,
  QUIET };            // blinks leds 10 times
Master NightwingSequence[] = {
  LISTENING_MY_TURN,// wait for 300ms pulse
  FIND_WALL,        // find wall (collision)
  DISCOVER_PATH,    // back up from wall until on top of path
  FOLLOW_PATH_1,    // follow path to find mine
  LISTENING_MINE_NIGHTWING,   // wait for 300ms response from command center
  FOLLOW_PATH_2,    // follow path to find end 
  FINAL_COLLISION,  // sends 300 ms, blinks led
  END,
  QUIET };
Master DanceSequence[] = {
  HALT,
  FORWARD_12,
  HALT,
  ROTATE_RIGHT_180, // arbitrary, could be LEFT
  HALT,
  BACK_3,
  HALT,
  TURN_LEFT,
  TURN_RIGHT_1,
  TURN_RIGHT_2,
  TURN_RIGHT_3, 
  // we are free to add more actions
  QUIET
};
Master TestBotSequence[] = { TEST_HALL_EFFECT, TEST_COLOR_DETECTION, TEST_ROTATION_TIME, TEST_TRANSMITTER, TEST_RECEIVER };
Master *MasterSequence = InitializeSequence; 
int MasterSequenceNum = 0;

enum Bot { SCARLET_WITCH, NIGHTWING, DANCER, TEST_BOT };  // NIGHTWING goes first, then SCARLET_WITCH
Bot BotType;

// dance routine
long DanceStepStartTime = 0;
boolean NotifyFinishedDanceStep = true;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);

  pinMode(GATE1, OUTPUT);
  pinMode(GATE2, OUTPUT);
  pinMode(GATE3, OUTPUT);
  pinMode(GATE4, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(LED_SENSOR_LEVEL, INPUT);

  pinMode(H_SENSOR, INPUT);
  pinMode(H_LED, OUTPUT);

  pinMode(FL->pin, OUTPUT);
  pinMode(FC->pin, OUTPUT);
  pinMode(FR->pin, OUTPUT);
  pinMode(B->pin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FL->pin), FL_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FC->pin), FC_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR->pin), FR_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B->pin),  B_bumper_event, CHANGE);

  pinMode(FL->ledPin, OUTPUT);
  pinMode(FC->ledPin, OUTPUT);
  pinMode(FR->ledPin, OUTPUT);
  pinMode(B->ledPin,  OUTPUT);

  pinMode(commsIn, INPUT);
  pinMode(commsAlert, INPUT);
  pinMode(commsOut, OUTPUT);
  pinMode(alertRed, OUTPUT);
  pinMode(alertYellow, OUTPUT);
  pinMode(alertBlue, OUTPUT);

  // generate 18.523 kHz when OCR3A=53 on Mega pin 5 (additional comments on trunk and lecture slides)
  pinMode(5, OUTPUT);
  TCCR3A = _BV(COM3A0) | _BV(COM3B0) | _BV(WGM30) | _BV(WGM31);
  TCCR3B = _BV(WGM32) | _BV(WGM33) |  _BV(CS31);
  OCR3A = 53; // higher numbers here mean lower out frequencies

  // set initial states
  
  CommsState = COMMS_LISTENING;
  PathState = PATH_FINDING;
  SearchSide = SEARCHING_LEFT;  
  ColorState = BLACK;
  MineState = NONE;
  halt();
}

// the loop routine runs over and over again forever:
// the loop is for changing the state if necessary, then executing the current state.
void loop() {
  switch (MasterSequence[MasterSequenceNum]) {
    case LOCKED:
      halt();
      drive();
      digitalWrite(alertYellow, HIGH);
      poll_bumpers();
      service_collisions();
      break;
    case SETTINGS:
      halt();
      drive();
      poll_bumpers();
      service_collisions();
      break;
    case APPLY_SETTINGS:
      halt();
      drive();
      poll_bumpers();
      service_collisions();
      switch(BotType) {
        case SCARLET_WITCH:
          MasterSequence = ScarletWitchSequence;
          MasterSequenceNum = 0;
          PathToFollow = RED;
          flash_led(alertRed, 100);
          break;
        case NIGHTWING:
          MasterSequence = NightwingSequence;
          MasterSequenceNum = 0;
          PathToFollow = BLUE;
          flash_led(alertBlue, 100);
          break;
        case DANCER:
          MasterSequence = DanceSequence;
          MasterSequenceNum = 0;
          flash_led(alertYellow, 100);
          break;
        case TEST_BOT:
          MasterSequence = TestBotSequence;
          MasterSequenceNum = 0;
          PathToFollow = RED;
          flash_led(alertRed, 50);
          flash_led(alertYellow, 50);
          flash_led(alertBlue, 50);
          break;
        default:
          ; // do nothing
      }
      break;
    case LISTENING_MY_TURN:         // listen for 300 ms from command center
      Serial.println("LISTENING_MY_TURN");
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case FIND_WALL:                 // find wall (collision)
      Serial.println("FIND_WALL");
      forward();
      poll_bumpers();
      service_collisions();
      drive();
      break;
    case DISCOVER_PATH:             // find the path from the wall
      Serial.println("DISCOVER_PATH");
      forward();
      detect_color();
      if (ColorState == PathToFollow) {
        MasterSequenceNum++; // found the path!
      }
      poll_bumpers();
      service_collisions();
      drive();      
      break;
    case FOLLOW_PATH_1:             // follow path to find mine
      Serial.println("FOLLOW_PATH_1");
      forward();
      detect_color();
      if (ColorState == RED)
        light_led(alertRed);
      else if (ColorState == BLUE)
        light_led(alertBlue);
      else
        light_led(alertYellow);
      follow_path();
      poll_bumpers();
      service_collisions();
      poll_h_sensor();
      service_h_sensor();
      drive();
      break;
    case LISTENING_MINE_SCARLET_WITCH:            // wait for 200 ms response from command center
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case LISTENING_MINE_NIGHTWING:              // wait for 400 ms response from command center
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case FOLLOW_PATH_2:             // follow path to find end
      forward();
      detect_color();
      follow_path();
      poll_bumpers();
      service_collisions();
      drive();
      break;
    case FINAL_COLLISION:           // blinks led
      send_message(PATH_END);
      send_message(PATH_END);
      MasterSequenceNum++;
      break;
    case FINAL_WAIT:                // waits for 400 hz from other bot
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case END: {                       // blinks leds 10 times
      halt();
      drive();
      for (int i = 0; i < 10; i++) {
        digitalWrite(alertRed, HIGH);
        digitalWrite(alertYellow, HIGH);
        digitalWrite(alertBlue, HIGH);
        delay(200);
        digitalWrite(alertRed, LOW);
        digitalWrite(alertYellow, LOW);
        digitalWrite(alertBlue, LOW);
        delay(200);
      }
    } break;
    case QUIET: {
      digitalWrite(alertRed, LOW);
      digitalWrite(alertYellow, LOW);
      digitalWrite(alertBlue, LOW);
      halt();
      drive();
    } break;
                                              // challenge 2 --------------------------------
    case HALT: {
      Serial.println("HALT");             
      halt();
      drive();
      listen_for_message();
    } break;
    case FORWARD_12: {
      Serial.println("FORWARD_12");
      forward();
      drive();
      listen_for_message();
    } break;
    case ROTATE_RIGHT_180: {
      Serial.println("ROTATE_RIGHT_180");
      turnRightInPlace();
      listen_for_message();
      drive();
    } break;
    case BACK_3: {
      Serial.println("BACK_3");
      reverse();
      listen_for_message();
      drive();
    } break;
    case TURN_LEFT: {
      Serial.println("TURN_LEFT");
      turnLeft();
      listen_for_message();
      drive();
    } break;
    case TURN_RIGHT_1: {
      Serial.println("TURN_RIGHT_1");
      long danceStepTime = 1000; // ms
      turnRight();
      listen_for_message();
      drive();
    } break;
    case TURN_RIGHT_2: {
      Serial.println("TURN_RIGHT_2");
      long danceStepTime = 1000; // ms
      turnRight();
      listen_for_message();
      drive();
    } break;
    case TURN_RIGHT_3: {
      Serial.println("TURN_RIGHT_3");
      turnRight();
      listen_for_message();
      drive();
    } break;
                                                  // test --------------------------------
    case TEST_TRANSMITTER:          // send each message, indicated with LEDs
      halt();
      drive();
      poll_bumpers();
      service_collisions();
      
      send_message(FOUND_MINE_SCARLET_WITCH);
      send_message(PATH_END);
      send_message(FOUND_MINE_NIGHTWING);
      break;
    case TEST_RECEIVER:             // recieve messages, indicated with LEDs
      halt();
      drive();
      poll_bumpers();
      service_collisions();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case TEST_ROTATION_TIME:
      turnRightInPlace();
      drive();
      delay(1475);
      halt();
      drive();
      delay(1000);
      turnLeftInPlace();
      drive();
      delay(750);
      halt();
      drive();
      delay(10000);
      break;
    case TEST_COLOR_DETECTION:
      Serial.println("TEST_COLOR_DETECTION");
      detect_color();
      if (ColorState == RED)
        light_led(alertRed);
      else if (ColorState == BLUE)
        light_led(alertBlue);
      else
        light_led(alertYellow);
      delay(700);
      break;
    case TEST_HALL_EFFECT:
      Serial.println("TEST_COLOR_DETECTION");
      halt();
      poll_h_sensor();
      service_h_sensor_test();
      drive();
    case TEST_PATH_FINDING:
      detect_color();
      follow_path();
      break;
  }
}

void notify_finished() {
  send_message(DANCE); // 400 ms
  send_message(DANCE);
  NotifyFinishedDanceStep = false;
}

void listen_for_message() {
  if (CommsState == COMMS_LISTENING) {
    poll_comms();
  }
  if (CommsState == COMMS_RECEIVING) {
    receive_message();
  }
}

void flash_led(int ledPin, int timeOn) {
  Serial.println("should be blinking");
  digitalWrite(ledPin, HIGH);
  delay(timeOn);
  digitalWrite(ledPin, LOW);
  delay(timeOn);
  digitalWrite(ledPin, HIGH);
  delay(timeOn);
  digitalWrite(ledPin, LOW);
}

void light_led(int ledPin) {
  digitalWrite(alertRed, LOW);
  digitalWrite(alertBlue, LOW);
  digitalWrite(alertYellow, LOW);
  digitalWrite(ledPin, HIGH);
}
