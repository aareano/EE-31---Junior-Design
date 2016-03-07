// ******************* COLLISION CONTROL ******************* //

// https://www.arduino.cc/en/Tutorial/Debounce
void pull_bumpers() {
  for(int i = 0; i < BumperCount; i++) {
    // if the input has settled and existed for long enough as the same value, use it!
    if ((millis() - Bumpers[i].lastDebounceTime) > BumperDebounceDelay) {
      Bumpers[i].pinState = digitalRead(Bumpers[i].pin) == HIGH ? true : false;
      
//      Serial.print(Bumpers[i].name);
//      Serial.print(" >> ");
//      Serial.println((int) Bumpers[i].pinState);
    }
  }
}

void service_collisions() {
  
  // update bumper states based on switch levels (pin states)
  for(int i = 0; i < BumperCount; i++) {     
    if (Bumpers[i].pinState) {  // bumper is depressed
      Serial.print(Bumpers[i].name);
      Serial.println(" is pressed");
      
      // bumper.state == DOWN           - should never happen here
      // bumper.state == DOWN_SERVICING - do nothing
      if (Bumpers[i].state == UP || Bumpers[i].state == UP_SERVICING) {
        Serial.print(Bumpers[i].name);
        Serial.println(" >> DOWN");
      
        Bumpers[i].state = DOWN;              // you hit a wall after not being on a wall.
      }
    } else {                // bumper is not depressed
//      Serial.print(Bumpers[i].name);
//      Serial.println(" is NOT pressed");

      // bumper.state == UP           - do nothing
      // bumper.state == DOWN         - should never happen here
      // bumper.state == UP_SERVICING - do nothing
      if (Bumpers[i].state == DOWN_SERVICING) {
        Serial.print(Bumpers[i].name);
        Serial.println(" >> UP_SERVICING");
        Bumpers[i].state == UP_SERVICING;     // we've moved off of the wall
      }
    }

//    Serial.print("Updated state of ");
//    Serial.print(Bumpers[i].name);
//    Serial.print(" :: ");
//    Serial.println(Bumpers[i].state);
  }

  // if two adjacent bumpers are down, service the collision with a double bumper
  if ((FL.state == DOWN || FL.state == DOWN_SERVICING) && (FC.state == DOWN || FC.state == DOWN_SERVICING)) {
    FL_FC.state = SERVICING;
//    Serial.print(FL_FC.name);
//    Serial.println(" >> SERVICING");
  }
  if ((FC.state == DOWN || FC.state == DOWN_SERVICING) && (FR.state == DOWN || FR.state == DOWN_SERVICING)) {
    FC_FR.state = SERVICING;
//    Serial.print(FC_FR.name);
//    Serial.println(" >> SERVICING");
  }
  if ((BL.state == DOWN || BL.state == DOWN_SERVICING) && (BR.state == DOWN || BR.state == DOWN_SERVICING)) {
    BL_BR.state = SERVICING;
//    Serial.print(BL_BR.name);
//    Serial.println(" >> SERVICING");
  }

  // service single bumpers
  for (int i = 0; i < BumperCount; i++) {
    // if the double bumper that encompansses this single bumper is SERVICING, then don't service the single bumper
    if (((FL_FC.children[0] == &Bumpers[i] || FL_FC.children[1] == &Bumpers[i]) && FL_FC.state == SERVICING)
      || ((FC_FR.children[0] == &Bumpers[i] || FC_FR.children[1] == &Bumpers[i]) && FC_FR.state == SERVICING)
      || ((BL_BR.children[0] == &Bumpers[i] || BL_BR.children[1] == &Bumpers[i]) && BL_BR.state == SERVICING)) {
      continue;
    }

    switch (Bumpers[i].state) {
      case UP:
        // nothing to service, do nothing
        break;
      case DOWN:  // we just had impact. start servicing.
        Bumpers[i].state = DOWN_SERVICING;
        Bumpers[i].timeTriggered = millis();
        Bumpers[i].service();
        break;
      case UP_SERVICING:    // cool, we're servicing. keep servicing.
        Bumpers[i].service();   
        break;
      case DOWN_SERVICING:  // cool, we're servicing. keep servicing.
        Bumpers[i].service();
        break;
    }
  }

  // service double bumpers
  for (int i = 0; i < DoubleBumperCount; i++) {
    switch (DoubleBumpers[i].state) {
      case SERVICING:
        DoubleBumpers[i].service();     // service!
        break;
      case SERVICED:
        // we're not serving now, so do nothing
        break;
    }
  }
}

// interrupt to update the time of the last voltage change on a switch
void FL_bumper_event() {
  Serial.println("FL bounced");
  FL.lastDebounceTime = millis();
}

void FC_bumper_event() {
  FC.lastDebounceTime = millis();
}

void FR_bumper_event() {
  FR.lastDebounceTime = millis();
}

void BL_bumper_event() {
  BL.lastDebounceTime = millis();
}

void BR_bumper_event() {
  BR.lastDebounceTime = millis();
}

// function to service the FL bumper when it is triggered individually
void service_FL() {
  Serial.println("Servcing FL");
  if (millis() < FL.timeTriggered + FL.serviceTime) { // service here
    digitalWrite(FL.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FL.ledPin, LOW);
    FL.state = UP;
  }
}


void service_FC() {
  if (millis() < FC.timeTriggered + FC.serviceTime) { // service here
    digitalWrite(FC.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FC.ledPin, LOW);
    FC.state = UP;
  }
}


void service_FR() {
  if (millis() < FR.timeTriggered + FR.serviceTime) { // service here
    digitalWrite(FR.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(FR.ledPin, LOW);
    FR.state = UP;
  }
}


void service_BL() {
  if (millis() < BL.timeTriggered + BL.serviceTime) { // service here
    digitalWrite(BL.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(BL.ledPin, LOW);
    BL.state = UP;
  }
}


void service_BR() {
  if (millis() < BR.timeTriggered + BR.serviceTime) { // service here
    digitalWrite(BR.ledPin, HIGH);
  } else { // the service is done. change states.
    digitalWrite(BR.ledPin, LOW);
    BR.state = UP;
  }
}

// double bumpers

void service_FL_FC() {
  if (millis() < FR.timeTriggered + FR.serviceTime) { // service here
    digitalWrite(FR.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(FR.ledPin, LOW);
    FL_FC.state = SERVICED;
    FL.state = UP;
    FC.state = UP;
  }
}


void service_FC_FR() {
  if (millis() < BL.timeTriggered + BL.serviceTime) { // service here
    digitalWrite(BL.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(BL.ledPin, LOW);
    FC_FR.state = SERVICED;
    FC.state = UP;
    FR.state = UP;
  }
}


void service_BL_BR() {
  if (millis() < BR.timeTriggered + BR.serviceTime) { // service here
    digitalWrite(BR.ledPin, HIGH);
  } else { // the service is done. change states (children too).
    digitalWrite(BR.ledPin, LOW);
    BL_BR.state = SERVICED;
    BL.state = UP;
    BR.state = UP;
  }
}

