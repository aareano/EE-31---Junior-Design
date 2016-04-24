
// ******************* COLOR SENSOR CONTROL ******************* //

void detect_color() {
  // illuminate blue LED
  if (PathToFollow == RED) {
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(BLUE_LED, HIGH);
  }
  delay(1);

  // check the sensor
  float reading = analogRead(LED_SENSOR_LEVEL);

  // turn off blue LED
  if (PathToFollow == RED) {
    digitalWrite(RED_LED, LOW);
  } else {
    digitalWrite(BLUE_LED, LOW);
  }

  Serial.print("reading: ");
  Serial.println(calcVolts(reading));

  if (calcVolts(reading) < BLUE_THRESHOLD) {
    ColorState = BLUE;
    Serial.println("BLUE");
  } else if (calcVolts(reading) < RED_THRESHOLD) {
    Serial.println("RED");
    ColorState = RED;
  } else { // BLACK
    Serial.println("BLACK");
    ColorState = BLACK;
  }
}

// scales 0-1023 to 0-5
float calcVolts(float analogIn) {
  return ( analogIn / 1023 ) * 5;
}


// ******************* PATH FOLLOWING ******************* //

void follow_path() {

  switch (PathState) {
    case PATH_FINDING:
      if (ColorState == PathToFollow) {
        PathState = ON_PATH;
      }
      forward();
      break;
    case ON_PATH:
      if (ColorState != PathToFollow) {
        //         we've just moved off the path
        PathState = RECOVERING;
        if (PathToFollow == RED) {
          SearchSide = SEARCHING_RIGHT;
        } else {
          SearchSide = SEARCHING_LEFT;
        }
        SearchStartTime = millis();
        SearchTime = BaseSearchTime;  // reset the search radius
      } else {
        forward();
      }
      break;
    case RECOVERING: {
        if ((millis() > SearchStartTime + MinSearchTime) && ColorState == PathToFollow) {
          PathState = ON_PATH;
          forward();
          return;
        } else if (millis() > SearchStartTime + SearchTime) { // search time expired
          SearchTime *= 2;
          SearchSide = SearchSide == SEARCHING_LEFT ? SEARCHING_RIGHT : SEARCHING_LEFT; // flip the search side
          SearchStartTime = millis();
        } else {
          if (SearchSide == SEARCHING_LEFT) {
            turnLeftInPlace();
          } else {
            turnRightInPlace();
          }
        }
      } break;
  }
}
