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
    if (BotType == SCARLET_WITCH) {
      send_message(FOUND_MINE_SCARLET_WITCH);
      send_message(FOUND_MINE_SCARLET_WITCH);  
    } else {
      send_message(FOUND_MINE_NIGHTWING);
      send_message(FOUND_MINE_NIGHTWING);  
    }
  } else if (MineState == NONE) {
    // do nothing
  }
}

void service_h_sensor_test() {
  if (MineState == FOUND) {
    digitalWrite(alertYellow, HIGH);
  } else if (MineState == NONE) {
    // do nothing
    digitalWrite(alertYellow, LOW);
  }
}
