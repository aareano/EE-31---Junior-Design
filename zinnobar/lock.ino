void check_combo() {
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
      return;
    }
  }
  // if we made it here, the combination is correct
  MasterSequenceNum++;
  
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