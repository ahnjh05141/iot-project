String input = "";

//const int LED = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Ready");
}

void loop() {
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("camera")) {
      Serial.println("📸 Capture command received");
    }
    else if (input.startsWith("segment")) {
      String num = input.substring(8);
      Serial.print("Segment Value: ");
      Serial.println(num);
      //digitalWrite(LED, HIGH);
    }
    else if (input.startsWith("lcd")) {
      String text = input.substring(4);
      Serial.print("LCD Text: ");
      Serial.println(text);
    }
  }
}
