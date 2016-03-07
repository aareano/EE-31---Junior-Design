

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
int GATE1 = 3;  // red
int GATE2 = 5;  // brown
int GATE3 = 6;  // orange
int GATE4 = 9;  // yellow

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
void service_BL();
void service_BR();
void service_FL_FC();
void service_FC_FR();
void service_BL_BR();

Bumper fl = { .pin = 2,  .pinState = false, .ledPin = 40, "FL", .state = UP, .timeTriggered = 0, .service = service_FL, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper fc = { .pin = 18, .pinState = false, .ledPin = 41, "FC", .state = UP, .timeTriggered = 0, .service = service_FC, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper fr = { .pin = 19, .pinState = false, .ledPin = 42, "FR", .state = UP, .timeTriggered = 0, .service = service_FR, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper bl = { .pin = 20, .pinState = false, .ledPin = 43, "BL", .state = UP, .timeTriggered = 0, .service = service_BL, .serviceTime = 1000, .lastDebounceTime = 0 };
Bumper br = { .pin = 21, .pinState = false, .ledPin = 44, "BR", .state = UP, .timeTriggered = 0, .service = service_BR, .serviceTime = 1000, .lastDebounceTime = 0 };

Bumper *FL = &fl;
Bumper *FC = &fc;
Bumper *FR = &fr;
Bumper *BL = &bl;
Bumper *BR = &br;

DoubleBumper fl_fc = { "FL_FC", .state = SERVICED, .service = service_FL_FC, .serviceTime = 1000, .children = { FL, FC } };
DoubleBumper fc_fr = { "FC_FR", .state = SERVICED, .service = service_FC_FR, .serviceTime = 1000, .children = { FC, FR } };
DoubleBumper bl_br = { "BL_BR", .state = SERVICED, .service = service_BL_BR, .serviceTime = 1000, .children = { BL, BR } };

DoubleBumper *FL_FC = &fl_fc;
DoubleBumper *FC_FR = &fc_fr;
DoubleBumper *BL_BR = &bl_br;

Bumper *Bumpers[] = { FL, FC, FR, BL, BR };
DoubleBumper *DoubleBumpers[] = { FL_FC, FC_FR, BL_BR };

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
  pinMode(BL->pin, OUTPUT);
  pinMode(BR->pin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FL->pin), FL_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FC->pin), FC_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(FR->pin), FR_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BL->pin), BL_bumper_event, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BR->pin), BR_bumper_event, CHANGE);

  pinMode(FL->ledPin, OUTPUT);
  pinMode(FC->ledPin, OUTPUT);
  pinMode(FR->ledPin, OUTPUT);
  pinMode(BL->ledPin, OUTPUT);
  pinMode(BR->ledPin, OUTPUT);

  pinMode(48, OUTPUT);  // temp LED for double bumper

  // set initial state
  ColorState = BLACK;
  MineState = NONE;
  
  PathState = PATH_FINDING;
  SearchSide = SEARCHING_LEFT;  
  PathToFollow = BLUE;
  halt();
}

// the loop routine runs over and over again forever:
// the loop is for changing the state if necessary, then executing the current state.
void loop() {

  // ** UPDATE THE CURRENT STATE (if necessary) ** //
  
  // check for collision
  poll_bumpers();

  // handle collision
  service_collisions();

  // check hall effect sensor
  poll_h_sensor();
  
  // check sound reciever(s)
    
  // check color sensor
  detect_color();
  
  // path following
  follow_path(PathToFollow);

  // ** EXECUTE THE CURRENT STATE ** //
  
  // execute mine state
  service_mine();

  // execute driving
  drive();  
  
  // execute (sound) communication
  
  // execute any other state-dependent actions (e.g. light LEDs)
    
  
  // ** EXECUTE STATE-INDEPENDENT ACTIONS (I can't think of any) ** //
  
//  Serial.println("------------------------");
}


