// ******************* COMMUNICATION CONTROL ******************* //

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

void serviceMessage() {
//  if (pulseCount >= 17 || pulseCount <= 23) {
  if (pulseCount >= 1 || pulseCount <= 3) {
    // message 1
    Serial.println("received message 1");
//  } else if (pulseCount >= 27 || pulseCount <= 33) {
  } else if (pulseCount >= 5 || pulseCount <= 6) {
    // message 2
    Serial.println("received message 2");
//  } else if (pulseCount >= 37 || pulseCount <= 43) {
  } else if (pulseCount >= 8 || pulseCount <= 12) {
    // message 3
    Serial.println("received message 3");
  } else {
    // we don't understand the message...
    Serial.println("help help!");
  }
  
  pulseCount = 0;  // reset pulseCount
}

// pulseIn() could be helpful here
//void listenToMessage() {
//  for (int i = 0; i < 50; i++) {
//    if (digitalRead(commsIn) == HIGH) {
//      pulseCount++;
//    }
//    delay(10);
//  }
//}

