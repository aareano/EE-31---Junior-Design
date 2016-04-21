// ******************* COMMUNICATION CONTROL ******************* //


// FYI - fast PWM is ouputting on pin 5 (see setup() in zinnobar.ino)

boolean poll_comms() {
  if (calcVolts(analogRead(commsIn)) > 1.0) {
    CommsState = COMMS_RECEIVING;
    return true;
  } else {
    CommsState = COMMS_LISTENING;
    return false;
  }
}

// ISR for the commsIn pin
// poll the input pin every 10 ms 50x (500ms total)
void receive_message() {
  int pulseCount = 0;
  for (int i = 0; i < 50; i++) {
    if (calcVolts(analogRead(commsIn)) > 1.0) {
      pulseCount++;
    }
    delay(10);
  }
  digitalWrite(commsAlert, LOW);
  Serial.print("pulse count: ");
  Serial.println(pulseCount);
  
  if (pulseCount >= 17 && pulseCount <= 23) {
    Serial.println("received 200 ms message");
    digitalWrite(alertRed, HIGH);
    delay(200);
    digitalWrite(alertRed, LOW);

    // MasterSequence = FOLLOWING_PATH;

//    if (MasterSequence == LISTENING_MY_TURN) {
//        MasterSequence = FINDING_PATH;
//    }
//    if (BotColor == NIGHTWING) {
//      if (MasterSequence == LISTENING_COMPANIONS_TURN) {
//        MasterSequence = FINAL_WAIT;
//      }
//    }
//    if (BotColor == SCARLET_WITCH) {
//      if (MasterSequence == LISTENING_COMPANIONS_TURN) {
//        MasterSequence = LISTENING_MY_TURN;
//      }
//    }
    
  } else if (pulseCount >= 27 && pulseCount <= 33) {
    Serial.println("received 300 ms message");

    digitalWrite(alertYellow, HIGH);
    delay(200);
    digitalWrite(alertYellow, LOW);
    
  } else if (pulseCount >= 37 && pulseCount <= 43) {
    Serial.println("received 400 ms message");
    digitalWrite(alertBlue, HIGH);
    delay(200);
    digitalWrite(alertBlue, LOW);
  } else {
    Serial.println("*** Received an unknown message in serviceMessage() ***");
    digitalWrite(alertRed, HIGH);
    digitalWrite(alertYellow, HIGH);
    digitalWrite(alertBlue, HIGH);
    delay(200);
    digitalWrite(alertRed, LOW);
    digitalWrite(alertYellow, LOW);
    digitalWrite(alertBlue, LOW);
  }

  CommsState = COMMS_LISTENING;
}

// message protocol is simply on-delay-off
void send_message(Message message) {
  int tempRSpeed = rightMotorSpeed;
  int tempLSpeed = leftMotorSpeed;
  noInterrupts();
  halt();
  drive();
  int offsetMs = 10;
  digitalWrite(commsOut, HIGH);
  switch (message) {
    case FOUND_MINE:
      delay(300 + offsetMs);
      break;
    case FINISHED:
      delay(400 + offsetMs);
      break;
    default:
      Serial.println("*** Trying to send an unknown message in sendMessageToCommandCenter() ***");
  }
  digitalWrite(commsOut, LOW);
  interrupts();
  delay(1500);
  rightMotorSpeed = tempRSpeed;
  leftMotorSpeed = tempLSpeed;
  drive();
}

// Challenge 2: The two bots communicate with each other sending commands to
// have one of the bots move forward twelve inches, stop, turn around 180 degrees, stop,
// move backwards three inches, stop, turn left, turn right, turn right, and turn right to end
// up as close as possible to the bot’s starting position. This test is ninety seconds.

// lasser is allowing us to do additional actions besides those specified in the challenge
