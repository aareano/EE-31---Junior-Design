// ******************* HALL EFFECT SENSOR CONTROL ******************* //

void poll_h_sensor() {
  float reading = calcVolts(analogRead(H_SENSOR));
//  Serial.print("pulling H sensor: ");
//  Serial.println(reading);
  if (reading < H_THRESHOLD) {
//    Serial.println("found mine");
    MineState = FOUND;
  } else {
    MineState = NONE;
  }
}

void service_mine() {
  if (MineState == FOUND) {
    digitalWrite(H_LED, HIGH);
  } else if (MineState == NONE) {
    digitalWrite(H_LED, LOW);
  }
}

