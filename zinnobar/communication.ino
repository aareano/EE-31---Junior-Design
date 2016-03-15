// ******************* COMMUNICATION CONTROL ******************* //

// an interrupt function for counting pulses
void receivedPulse() {  
  Serial.println("receivedPulse()");
  if (receivingMessage) {
    pulseCount++;
  } else {
    receivingMessage = true;
    messageStartTime = millis();
    pulseCount = 1;
  }
}


// respond to a message, depending on how many pulses we received
void serviceMessage() {
//  if (pulseCount >= 17 && pulseCount <= 23) {
  if (pulseCount >= 1 && pulseCount <= 3) {
    // message 1
    Serial.print("received message 1");
//  } else if (pulseCount >= 27 && pulseCount <= 33) {
  } else if (pulseCount >= 5 && pulseCount <= 6) {
    // message 2
    Serial.print("received message 2");
//  } else if (pulseCount >= 37 && pulseCount <= 43) {
  } else if (pulseCount >= 8 && pulseCount <= 12) {
    // message 3
    Serial.print("received message 3");
  } else {
    // we don't understand the message...
    Serial.print("help help!");
  }
  Serial.print(" - pulseCount: ");
  Serial.println(pulseCount);
  pulseCount = 0;  // reset pulseCount
}

// send a square wave (message) with a given frequency (hz) and a given number of pulses (depending on messageNum)
void sendMessage(float hz, int messageNum) {
  int tempRSpeed = rightMotorSpeed;
  int tempLSpeed = leftMotorSpeed;
  halt();   // don't want to be moving while transmitting

  float timeSteady = (1 / hz / 2) * 1000;    // assuming 50% duty cycle
  int transmitTime = 6000;         // 600 ms
  int startTime = millis();
  int level = 0;
  Serial.print("timeSteady - ");
  Serial.println(timeSteady);
  Serial.print("transmitTime - ");
  Serial.println(transmitTime);
  for (int pulses = 0; millis() - startTime < transmitTime; pulses++) {
    if (level == 0) {
      digitalWrite(commsOut, HIGH);
      level = 1;
      delay(timeSteady);
    } else {
      digitalWrite(commsOut, LOW);
      level = 0;
      delay(timeSteady);
    }
  }

  // set speed back to what it was before
  rightMotorSpeed = tempRSpeed;
  leftMotorSpeed = tempLSpeed;
}

