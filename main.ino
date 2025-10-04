#include <Arduino.h>
#include <math.h>
#include <LiquidCrystal.h>

// 핀 설정
const int BUZZER_PIN = 8;
const int BUTTON_PIN = 2;
const int SEGMENT_PINS[] = {3, 4, 5, 6, 7, 9, 10, 11};
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// 전역 변수
int currentSongIndex = 0;
bool isPlaying = false;
bool noteActive = false;

// 버튼 디바운스 변수
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// 노래 정보 설정
Note song[] = {
  Note("Bb4", 2.0), Note("Bb4", 1.0), Note("C5", 1.0),
  Note("Bb4", 1.0), Note("A4", 0.75), Note("G4", 0.25), Note("F4", 2.0),
  Note("Bb3", 1.5), Note("C4", 0.5), Note("D4", 1.0), Note("Bb3", 1.0),
  Note("F4", 3.0), Note("R", 1.0),

  Note("G4", 1.5), Note("F4", 0.5), Note("Eb4", 2.0),
  Note("Bb4", 1.0), Note("A4", 0.75), Note("G4", 0.25), Note("F4", 2.0),
  Note("F4", 1.5), Note("G4", 0.5), Note("A4", 1.0), Note("Bb4", 1.0),
  Note("C5", 3.0), Note("F4", 1.0),

  Note("D5", 2.0), Note("D5", 1.0), Note("C5", 1.0),
  Note("Bb4", 1.0), Note("A4", 1.0), Note("G4", 2.0),
  Note("C5", 2.0), Note("C5", 1.0), Note("Bb4", 1.0),
  Note("A4", 1.0), Note("G4", 1.0), Note("F4", 1.0), Note("D4", 0.75), Note("C4", 0.25),

  Note("Bb3", 2.0), Note("R", 0.25), Note("Bb4", 0.25), Note("Bb4", 0.25), Note("A4", 0.25),
  Note("G4", 2.0), Note("R", 1.0), Note("G4", 1.0),
  Note("F4", 2.0), Note("G4", 1.0), Note("A4", 1.0),
  Note("Bb4", 3.0), Note("R", 1.0),

  Note("D4", 0.75), Note("Eb4", 0.25), Note("F4", 0.5), Note("R", 0.5), Note("F4", 2.0),
  Note("Eb4", 0.75), Note("F4", 0.25), Note("G4", 0.5), Note("R", 0.5), Note("G4", 2.0),
  Note("F4", 1.5), Note("Bb4", 0.5), Note("Bb4", 1.0), Note("C5", 1.0),
  Note("D5", 3.0), Note("D5", 1.0),

  Note("D5", 2.0), Note("D5", 0.5), Note("D5", 0.5), Note("C5", 0.5), Note("D5", 0.5),
  Note("Eb5", 3.0), Note("Eb5", 0.75), Note("Eb5", 0.25),
  Note("F4", 1.5), Note("G4", 0.5), Note("A4", 1.0), Note("C5", 1.0),
  Note("Bb4", 3.0), Note("R", 1.0)
};

const int BPM = 160;
const int songLength = sizeof(song) / sizeof(song[0]);


// 세그먼트 디스플레이에 음표 이름 표시
void displayLetter(const char* noteName) {
  bool segments[7] = {false, false, false, false, false, false, false};

  for (int i = 0; i < 7; i++) { digitalWrite(SEGMENT_PINS[i], LOW); }
  digitalWrite(SEGMENT_PINS[7], LOW);

  char noteChar = noteName[0];
  switch (noteChar) {
    case 'A':
      segments[0] = true; segments[1] = true; segments[2] = true; segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 'B':
      segments[1] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 'C':
      segments[0] = true; segments[3] = true; segments[4] = true; segments[5] = true;
      break;
    case 'D':
      segments[1] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[6] = true;
      break;
    case 'E':
      segments[0] = true; segments[3] = true; segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 'F':
      segments[0] = true; segments[4] = true; segments[5] = true; segments[6] = true;
      break;
    case 'G':
      segments[0] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[5] = true;
      break;
    default: break;
  }

  for (int i = 0; i < 7; i++) {
    digitalWrite(SEGMENT_PINS[i], segments[i] ? HIGH : LOW);
  }

  if (strchr(noteName, 'b') != NULL) {
    digitalWrite(SEGMENT_PINS[7], HIGH);
  } else {
    digitalWrite(SEGMENT_PINS[7], LOW);
  }
}

// 악보의 음표 클래스
class Note {
  private:
    const char* name;   // 계이름
    float beats;        // 박자 (마디 기준)
    unsigned long startTime = 0; // 노트가 시작한 시간
    unsigned long duration = 0;  // 노트의 지속 시간(ms)

    int letterToSemitone(char N) {
      // 알파벳 계이름을 미디 수치로 변환합니다.
      switch (N) {
        case 'C': return 0; case 'D': return 2; case 'E': return 4;
        case 'F': return 5; case 'G': return 7; case 'A': return 9;
        case 'B': return 11;
      }
      return 0;
    }

  public:
    Note(const char* n, float b) {
      name = n;
      beats = b;
    }

    float getBeats() {
      // Getter: 재생할 시간(박자)를 반환합니다.
      return beats;
    }

    unsigned int getNoteFreq() {
      // Getter: 주파수를 반환합니다.
      if (name[0] == 'R') return 0;

      char N = name[0];
      bool sharp = (strchr(name, '#') != NULL);
      int octave = name[strlen(name) - 1] - '0';

      int offset = letterToSemitone(N) + (sharp ? 1 : 0);
      int midi = 12 * (octave + 1) + offset;
      double f = 440.0 * pow(2.0, (midi - 69) / 12.0);
      return (unsigned int)(f + 0.5);
    }

    // 논블로킹 시작: 노트를 시작할 때 한 번 호출
    void start(int buzzerPin, int bpm) {
      unsigned int freq = getNoteFreq();
      lcd.setCursor(0, 0);
      lcd.print("Freq: ");
      lcd.print(freq);
      lcd.print("   ");
      displayLetter(name);
      int beatDuration = 60000 / bpm;
      duration = (unsigned long)(beats * beatDuration);
      startTime = millis();
      if (freq > 0) {
        tone(buzzerPin, freq);
      } else {
        noTone(buzzerPin);
      }
    }

    // 논블로킹 업데이트: 반복 호출, 노트가 끝나면 true 반환
    bool update(int buzzerPin) {
      if (duration == 0) return true; // 시작 안됨
      if (millis() - startTime >= duration) {
        noTone(buzzerPin);
        duration = 0;
        return true;
      }
      return false;
    }
};


void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  for (int i = 0; i < 8; i++) {
    pinMode(SEGMENT_PINS[i], OUTPUT);
    digitalWrite(SEGMENT_PINS[i], LOW);
  }
  // Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.clear();
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    static int buttonState = HIGH;
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) { isPlaying = !isPlaying; }
    }
  }
  lastButtonState = reading;

  if (isPlaying) {
    if (!noteActive) {
      song[currentSongIndex].start(BUZZER_PIN, BPM);
      noteActive = true;
    } else {
      if (song[currentSongIndex].update(BUZZER_PIN)) {
        currentSongIndex = (currentSongIndex + 1) % songLength;
        noteActive = false;
      }
    }
  } else {
    noTone(BUZZER_PIN);
    noteActive = false;
  }
}