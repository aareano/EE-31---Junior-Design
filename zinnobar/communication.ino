// ******************* COMMUNICATION CONTROL ******************* //


// FYI - fast PWM is ouputting on pin 5 (see setup() in zinnobar.ino)


// ISR for commsIN pin - notifies bot that we recieved an inital impulse
void receivedPulse() {
  receiveMessage();
}

// poll the input pin every 10 ms 50x (500ms total)
// count the pulses that occur in that amount of time
void receiveMessage() {
  int pulseCount = 0;
  for (int i = 0; i < 50; i++) {
    if (digitalRead(commsIn) == HIGH) {
      pulseCount++;
    }
    delay(10);
  }
  serviceMessage(pulseCount);
}

// respond to a message, depending on how many pulses we received
void serviceMessage(pulseCount) {
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

// message protocol is simply on-delay-off
void sendMessageToCommandCenter(CommandCenterMessage message) {
  digitalWrite(commsOut, HIGH);
  switch (message) {
    case BEGIN:
      delay(200);   // not sure if these delay amounts are right
      break;
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

// message protocol is TBD
void sendMessageToBot(BotMessage message) {
  digitalWrite(commsOut, HIGH);
    switch (message) {
    case MOVE_FORWARD:
      delay(200);       // not sure how we're doing this protocol yet...
      break;
    case MOVE_BACKWARD:
      delay(300);
      break;
    case TURN_RIGHT:
      delay(400);
      break;
    case TURN_LEFT:
      delay(400);
      break;
    default:
      Serial.println("*** Trying to send an unknown message in sendMessageToBot() ***");
  }
  digitalWrite(commsOut, LOW);
}
