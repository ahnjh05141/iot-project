#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const String WHOAMI = "20206175 AJH";
const int BUZZ = 10;

void receiveLCD(String msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WHOAMI);
  lcd.setCursor(0, 1);
  lcd.print(msg);
  Serial.print("LCD Display: ");
  Serial.println(msg);
}

void receiveSegment(int num) {
  int segPins[] = {2, 3, 4, 5, 6, 7, 8};
  int digits[10][7] = {
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}  // 9
  };

  if (num < 0 || num > 9) return;

  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
    digitalWrite(segPins[i], digits[num][i]);
  }

  Serial.print("7 Segments: ");
  Serial.println(num);
}

void recieveCamera() {
  tone(BUZZ, 392);
  delay(200);
  tone(BUZZ, 262);
  delay(200);
  tone(BUZZ, 392);
  delay(200);
  tone(BUZZ, 262);
  delay(200);
  noTone(BUZZ);
  
  Serial.println("Camera Captured");
}

void setup() {
  pinMode(BUZZ, OUTPUT);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WHOAMI);
  Serial.println("Arduino Ready");
}

String input = "";

void loop() {
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("camera")) {
      Serial.println("📸 Capture command received");
      recieveCamera();
    }
    else if (input.startsWith("segment")) {
      String num = input.substring(8);
      int number = num.toInt();
      receiveSegment(number);
    }
    else if (input.startsWith("lcd")) {
      String text = input.substring(4);
      receiveLCD(text);
    }
  }
}