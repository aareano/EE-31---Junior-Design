// ******************* COMMUNICATION CONTROL ******************* //


// FYI - fast PWM is ouputting on pin 5 (see setup() in zinnobar.ino)


// ISR for commsIN pin - notifies bot that we recieved an inital impulse
void receivedPulse() {
  receiveMessageFromCommandCenter();
}

// poll the input pin every 10 ms 50x (500ms total)
// count the pulses that occur in that amount of time
void receiveMessageFromCommandCenter() {
  int pulseCount = 0;
  for (int i = 0; i < 50; i++) {
    if (digitalRead(commsIn) == HIGH) {
      pulseCount++;
    }
    delay(10);
  }
  serviceCommandCenterMessage(pulseCount);
}

void receiveMessageFromBot() {
  int pulseCount = 0;
  for (int i = 0; i < 50; i++) {
    if (digitalRead(commsIn) == HIGH) {
      pulseCount++;
    }
    delay(10);
  }
  serviceBotMessage(pulseCount);
}

// respond to a message, depending on how many pulses we received
void serviceCommandCenterMessage(int pulseCount) {
  if (pulseCount >= 17 && pulseCount <= 23) {
    // message 1
    Serial.println("received message 1");
  } else if (pulseCount >= 27 && pulseCount <= 33) {
    // message 2
    Serial.println("received message 2");
  } else if (pulseCount >= 37 && pulseCount <= 43) {
    // message 3
    Serial.println("received message 3");
  } else {
    Serial.println("*** Received an unknown message in serviceMessage() ***");
  }
}

void serviceBotMessage(int pulseCount) {
  if (pulseCount >= 27 && pulseCount <= 33) {
    // message 1
    // TODO: step through the BotSequence { MOVE_FORWARD_12, MOVE_FORWARD_15, MOVE_BACKWARD_3, TURN_RIGHT, TURN_LEFT, TURN_180 }
    Serial.println("received message from companion bot!");
  }
}

// message protocol is simply on-delay-off
void sendMessageToCommandCenter(CommandCenterMessage message) {
  digitalWrite(commsOut, HIGH);
  switch (message) {
    case FOUND_MINE:
      delay(300);
      break;
    case FINISHED:
      delay(400);
      break;
    default:
      Serial.println("*** Trying to send an unknown message in sendMessageToCommandCenter() ***");
  }
  digitalWrite(commsOut, LOW);
}

// Challenge 2: The two bots communicate with each other sending commands to
// have one of the bots move forward twelve inches, stop, turn around 180 degrees, stop,
// move backwards three inches, stop, turn left, turn right, turn right, and turn right to end
// up as close as possible to the bot’s starting position. This test is ninety seconds.

// lasser is allowing us to do additional actions besides those specified in the challenge

void sendMessageToBot() {
  digitalWrite(commsOut, HIGH);
  delay(300);
  digitalWrite(commsOut, LOW);
}
