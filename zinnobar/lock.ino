void check_combo() {
  for (int i = 0; i < comboLength; i++) { 
    Serial.print(userCombo[i]);
    Serial.print(", ");
  }
  for (int i = 0; i < comboLength; i++) {
    if (correctCombo[i] != userCombo[i]) {
      digitalWrite(alertRed, HIGH);
      delay(250);
      digitalWrite(alertRed, LOW);
      delay(250);
      digitalWrite(alertRed, HIGH);
      delay(250);
      digitalWrite(alertRed, LOW);
      lockInputNum = 0; // reset the user input
      Serial.print("Incorrect combination (at position ");
      Serial.print(i);
      Serial.println(")");
      return;
    }
  }
  // if we made it here, the combination is correct
  MasterSequenceNum++;
  Serial.println("Correct combination");
  // turn off the yellow led that was turned on in loop()
  digitalWrite(alertYellow, LOW);

  // flash blue led to user
  digitalWrite(alertBlue, HIGH);
  delay(250);
  digitalWrite(alertBlue, LOW);
  delay(250);
  digitalWrite(alertBlue, HIGH);
  delay(250);
  digitalWrite(alertBlue, LOW);

}
