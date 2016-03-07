// ******************* HALL EFFECT SENSOR CONTROL ******************* //

void pull_h_sensor() {
  float reading = calcVolts(analogRead(H_SENSOR));
//  Serial.print("pulling H sensor: ");
//  Serial.println(reading);
  if (reading < H_THRESHOLD) {
    Serial.println("found mine");
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

// ******************* COLOR SENSOR CONTROL ******************* //

void detect_color() {
  // illuminate blue LED
  digitalWrite(BLUE_LED, HIGH);
  delay(1);

  // check the sensor
  float reading = analogRead(LED_SENSOR_LEVEL);
  
  // turn off blue LED
  digitalWrite(BLUE_LED, LOW);

  Serial.print("reading: ");
  Serial.println(calcVolts(reading));
  
  if (calcVolts(reading) < BLUE_THRESHOLD) {
    ColorState = BLUE;
  } else if (calcVolts(reading) < RED_THRESHOLD) {
    ColorState = RED;
  } else { // BLACK
    ColorState = BLACK;
  }
  
  Serial.print("ColorState: ");
  Serial.println(ColorState);
}

// scales 0-1023 to 0-5
float calcVolts(float analogIn) {
  return ( analogIn / 1023 ) * 5;
}
