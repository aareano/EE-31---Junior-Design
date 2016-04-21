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

void service_h_sensor() {
  if (MineState == FOUND) {
    MasterSequenceNum++;
    send_message(FOUND_MINE);
    send_message(FOUND_MINE);
  } else if (MineState == NONE) {
    // do nothing
  }
}
