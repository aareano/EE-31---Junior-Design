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
    flash_led(alertRed, 250);

    switch(MasterSequence[MasterSequenceNum]) {
        case LISTENING_MINE_SCARLET_WITCH:
          MasterSequenceNum++;
          break;
    }
    
  } else if (pulseCount >= 27 && pulseCount <= 33) {
    Serial.println("received 300 ms message");
    flash_led(alertYellow, 250);

    switch(MasterSequence[MasterSequenceNum]) {
        case LISTENING_MY_TURN:
          MasterSequenceNum++;   // progress to the next state in the sequence
          break;
        case FINAL_WAIT:
          MasterSequenceNum++;
          break;
    }
    
  } else if (pulseCount >= 37 && pulseCount <= 43) {
    Serial.println("received 400 ms message");
    flash_led(alertBlue, 250);
    
    switch(MasterSequence[MasterSequenceNum]) {
        case LISTENING_MINE_NIGHTWING:
          MasterSequenceNum++;
          break;
                                // challenge 2 ---------------------
        case HALT:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case FORWARD_12:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case ROTATE_RIGHT_180:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case BACK_3:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case TURN_LEFT:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case TURN_RIGHT_1:
          MasterSequenceNum++;
          DanceStepStartTime = millis();
          NotifyFinishedDanceStep = true;
          break;
        case TURN_RIGHT_2:
          MasterSequenceNum++;
          break;
    }

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
  halt();
  drive();
  int offsetMs = 10;
  digitalWrite(commsOut, HIGH);
  switch (message) {
    case FOUND_MINE_SCARLET_WITCH:
      digitalWrite(alertYellow, HIGH);
      delay(200 + offsetMs);
      digitalWrite(alertYellow, LOW);
      break;
    case PATH_END:
      digitalWrite(alertRed, HIGH);
      delay(300 + offsetMs);
      digitalWrite(alertRed, LOW);
      break;
    case FOUND_MINE_NIGHTWING:
      digitalWrite(alertBlue, HIGH);
      delay(400 + offsetMs);
      digitalWrite(alertBlue, LOW);
      break;
    case DANCE:
      digitalWrite(alertBlue, HIGH);
      delay(400 + offsetMs);
      digitalWrite(alertBlue, LOW);
      break;
    default:
      Serial.println("*** Trying to send an unknown message in sendMessageToCommandCenter() ***");
  }
  digitalWrite(commsOut, LOW);
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
