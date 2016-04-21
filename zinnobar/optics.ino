
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
      // Serial.println("path finding...");
      if (ColorState == PathToFollow) {
        PathState = ON_PATH;
      }
      forward_slow();
      break;
    case ON_PATH:
      // Serial.println("on path!");
      if (ColorState != PathToFollow) { // we've just moved off the path
        PathState = SEARCHING_FIRST_SIDE;
        SearchStartTime = millis();
      }
      SearchTime = BaseSearchTime;  // reset the search radius
      forward_slow();
      break;
    case SEARCHING_FIRST_SIDE:
      // Serial.print("searching first side - ");
      // Serial.println(SearchSide == SEARCHING_LEFT ? "SEARCHING_LEFT" : "SEARCHING_RIGHT");

      if ((millis() - SearchStartTime > MinSearchTime) && ColorState == PathToFollow) {
        // Serial.println("checking for path!!!!!!!!!!!");
        PathState = ON_PATH; // check at the end search time
      }
      
      if (millis() - SearchStartTime > SearchTime) { // search time expired
        
        PathState = SEARCHING_SECOND_SIDE;
        SearchSide = SearchSide == SEARCHING_LEFT ? SEARCHING_RIGHT : SEARCHING_LEFT; // flip the search side
        SearchStartTime = millis();
      } else {
        if (SearchSide == SEARCHING_LEFT) {
          turnLeftInPlace();
        } else {
          turnRightInPlace();
        }
      }
      break;
    case SEARCHING_SECOND_SIDE:
      // Serial.print("searching second side - ");
      // Serial.println(SearchSide == SEARCHING_LEFT ? "SEARCHING_LEFT" : "SEARCHING_RIGHT");

      if ((millis() - SearchStartTime > MinSearchTime) && ColorState == PathToFollow) {
        PathState = ON_PATH; // check at the end search time
      }
      
      if (millis() - SearchStartTime > (2 * SearchTime)) { // search time expired

        SearchTime *= 1.5;  // increase the radius by a factor of 1.5
        PathState = SEARCHING_FIRST_SIDE;
        SearchSide = SearchSide == SEARCHING_LEFT ? SEARCHING_RIGHT : SEARCHING_LEFT; // flip the search side
        SearchStartTime = millis();
      } else {
        if (SearchSide == SEARCHING_LEFT) {
          turnLeftInPlace();
        } else {
          turnRightInPlace();
        }
      }
      break;
  }
}
