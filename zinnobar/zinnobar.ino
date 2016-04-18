
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
int GATE3 = 11;  // grey
int GATE4 = 10;  // purple

enum MotorName { LEFT, RIGHT };

// optic sensor
int BLUE_LED = 51;

int LED_SENSOR_LEVEL = A1;

// Maximum voltage readings for that color (with the appropriate LED lit)
float BLUE_THRESHOLD = 4.6;
float RED_THRESHOLD = 4.9;

// represents the color the bot has most recently detected
enum Color { BLACK, BLUE, RED };
Color ColorState;

// Line Following
enum Path { PATH_FINDING, SEARCHING_FIRST_SIDE, SEARCHING_SECOND_SIDE, ON_PATH };
enum Side { SEARCHING_LEFT, SEARCHING_RIGHT };
Color PathToFollow;
Side SearchSide;
Path PathState;
long BaseSearchTime = 200;
long SearchTime = 200;      // this is how long to search for the line on the left or right when we get off of it
long MinSearchTime = 75;   // the bot needs to turn at least this much before it starts checking for the path again
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
enum Message { BEGIN, FOUND_MINE, FINISHED, COMPANION_MOVE };
enum Comms { COMMS_LISTENING, COMMS_RECEIVING };
Comms CommsState;

enum Master { LISTENING_MY_TURN, LISTENING_COMPANIONS_TURN, FOLLOWING_PATH, FINISH_LINE, FINAL_WAIT, FINAL_ALIGNMENT, END };
Master MasterState;
enum Bot { NIGHTWING, SCARLET_WITCH };  // NIGHTWING goes first, then SCARLET_WITCH
Bot BotColor;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);

  pinMode(GATE1, OUTPUT);
  pinMode(GATE2, OUTPUT);
  pinMode(GATE3, OUTPUT);
  pinMode(GATE4, OUTPUT);

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

  pinMode(48, OUTPUT);  // temp LED for double bumper

  pinMode(commsOut, OUTPUT);
  pinMode(commsIn, INPUT);
  pinMode(commsAlert, INPUT);


  // generate 18.523 kHz when OCR3A=53 on Mega pin 5 (additional comments on trunk and lecture slides)
  pinMode(5, OUTPUT);
  TCCR3A = _BV(COM3A0) | _BV(COM3B0) | _BV(WGM30) | _BV(WGM31);
  TCCR3B = _BV(WGM32) | _BV(WGM33) |  _BV(CS31);
  OCR3A = 53; // higher numbers here mean lower out frequencies

  // set initial states
  
  BotColor = NIGHTWING;   // eventually turn this into a switch on the bot
  if (BotColor == NIGHTWING) {
    MasterState = LISTENING_MY_TURN;
    PathToFollow = BLUE;
  } else if (BotColor == SCARLET_WITCH) {
    MasterState = LISTENING_COMPANIONS_TURN;
    PathToFollow = RED;
  }
  
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
  boolean testing = false;
  if (testing) {
    drive();
//    poll_bumpers();
//    service_collisions();
    if (CommsState == COMMS_LISTENING) {
        poll_comms();
    }
    if (CommsState == COMMS_RECEIVING) {
      receive_message();
    }
    return;
  }
  
  switch (MasterState) {
    case LISTENING_MY_TURN:
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case LISTENING_COMPANIONS_TURN:
      halt();
      drive();
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      break;
    case FOLLOWING_PATH:
      if (CommsState == COMMS_LISTENING) {
        poll_comms();
      }
      if (CommsState == COMMS_RECEIVING) {
        receive_message();
      }
      detect_color();
      follow_path(PathToFollow);
      poll_bumpers();
      service_collisions();
      poll_h_sensor();
      service_h_sensor();
      drive();
      break;
    case FINISH_LINE:
      detect_color();
      follow_path(PathToFollow);
      poll_bumpers();
      service_collisions();
      drive();
      break;
    case END:
      halt();
      drive();
      break;
  }
}
