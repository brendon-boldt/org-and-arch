#include <LiquidCrystal.h>
#include <stdbool.h>
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

const int contrastPin = 9;
const int button1Pin = 12;
const int button2Pin = 13;
const int button3Pin = 11;
const int potPin = A5;
const int buzzerPin = 8;

int curScreen = 0;
int contrast = 0;

// Easily keep track of button states
typedef struct {
  int cur;
  int prev;
} ButtonState;

ButtonState b1, b2, b3;

bool press1 = false,
  press2 = false,
  press12 = false,
  press3 = false;

// Store the type of press as a binary number where each bit
// represents a button press. This allows for multi-button presses
// 0 - none, 1 - b1, 2 - b2, 3 - b1 & b2, 4 - b3
int pressType = 0;

// Keep track of whether the screen needs updating
bool updateScreen = true;

bool tunerOn = false;
double tunerBase = 440.0;
// How many half steps the tone is away from A4 (~440Hz)
int tunerPitch = 0;

// The names (A, A#, etc.) of the pitches
const char * pitchNames[12];
bool metronomeOn = false;
int metronomeTempo = 100;
// Number of beats per measure
int metronomeBeats = 1;
// Which beat the metronome is on
int beatNumber = 0;
bool playBeat = false;
// For keeping track of tempo
long long currentTime;
long long oldTime;

void initializePitchNames() {
  pitchNames[0] = "A";
  pitchNames[1] = "A#";
  pitchNames[2] = "B";
  pitchNames[3] = "C";
  pitchNames[4] = "C#";
  pitchNames[5] = "D";
  pitchNames[6] = "D#";
  pitchNames[7] = "E";
  pitchNames[8] = "F";
  pitchNames[9] = "F#";
  pitchNames[10] = "G";
  pitchNames[11] = "G#";
}

void tunerScreen() {
  if (press3) {
    tunerOn = !tunerOn;
  } else if (press1) {
    --tunerPitch;
    updateScreen = true;
  } else if (press2) {
    ++tunerPitch;
    updateScreen = true;
  } else if (press12) {
    tunerOn = false;
  }

  double oldBase = tunerBase;
  // Use the potentiometer to decided the base pitch
  int potPitch = 6*analogRead(potPin)/1024;

  switch (potPitch) {
    case 0: tunerBase = 415.0; break; // Baroque!
    case 1: tunerBase = 430.0; break; // "Classical"
    case 2: tunerBase = 440.0; break; // "Standard"
    case 3: tunerBase = 441.0; break; // Meh
    case 4: tunerBase = 442.0; break; // Meh
    case 5: tunerBase = 443.0; break; // Just right
    default: tunerBase = 443.0;
  }

  // Display the new base if it has changed.
  if (oldBase != tunerBase)
    updateScreen = true;
  

  if (tunerOn) {
    // I'll just use equal temperament because it is easy
    tone(buzzerPin, tunerBase * pow(2.0, tunerPitch/12.0));
  } else {
    noTone(buzzerPin);
  }

  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Base: ");
    lcd.print((int) tunerBase);
    lcd.print("Hz");
    lcd.setCursor(0,1);
    lcd.print("Note: ");
    lcd.print(pitchNames[(1200+tunerPitch) % 12]);
    // This is the logic to determine the number that comes after
    // the pitch name.
    lcd.print((57+tunerPitch)/12);
    updateScreen = false;
  }
}

// Print out a line showing which beat the metronome is on
void printBeat(int num, int index) {
  lcd.setCursor(0,1);
  for (int i = 0; i < num; ++i) {
    if (i == index) {
      lcd.write(255);
      lcd.print(" ");
    } else {
      lcd.print(". ");
    }
  }
}

void metronomeScreen() {
  currentTime = millis();
  if (press3) {
    metronomeOn = !metronomeOn;
    playBeat = metronomeOn;
  } else if (press1) {
    metronomeTempo -= 2;
    updateScreen = true;
  } else if (press2) {
    metronomeTempo += 2;
    updateScreen = true;
  } else if (press12) {
    metronomeOn = false;
  }

  int oldBeats = metronomeBeats;
  // Use the potentiometer to decide the beats per measure
  metronomeBeats = 1 + 4*analogRead(potPin)/1024;
  if (metronomeBeats != oldBeats)
    updateScreen = true;

  // Play a new beat every so often
  // where "so often" is decided by the tempo
  if (metronomeOn && currentTime - oldTime > (60000.0/metronomeTempo)) {
    playBeat = true;
  }

  if (playBeat) {
    // For the first beat, play a higher pitch
    if (beatNumber == 0) {
      tone(buzzerPin, 660, 50);
    } else {
      tone(buzzerPin, 440, 50);
    }
    updateScreen = true;
    oldTime = currentTime;
  }

  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tempo: ");
    lcd.print(metronomeTempo);
    lcd.print("bpm");
    printBeat(metronomeBeats, beatNumber);
    updateScreen = false;
  }

  // This has to updated after the display logic in order for the
  // LCD to match the sound
  if (playBeat) {
    beatNumber = (beatNumber + 1) % metronomeBeats;
    playBeat = false;
  }

}

// Determine if the button was just recently released
bool isReleased(ButtonState * b) {
  return b->cur == HIGH && b->prev == LOW;
}

void checkButtons() {
  b1.cur = digitalRead(button1Pin);
  b2.cur = digitalRead(button2Pin);
  b3.cur = digitalRead(button3Pin);

  // Use ing OR-equals allows for different buttons to be
  // registered as they pressed sequentially (if the first ones
  // remain held down.
  if (b1.cur == LOW)
    pressType |= 1;
  if (b2.cur == LOW)
    pressType |= 2;
  if (b3.cur == LOW)
    pressType |= 4;

  switch (pressType) {
    case 0:
      break;
    case 1:
      if (isReleased(&b1)) {
        // Debouncing
        bool debounce = true;
        for (int i = 0; i < 8; ++i) {
          delay(10);
          debounce &= (digitalRead(button1Pin) == HIGH) ? true : false;
        }
        if (debounce) {
          press1 = true;
          pressType = 0;
        }
      }
      break;
    case 2:
      if (isReleased(&b2)) {
        // Debouncing
        bool debounce = true;
        for (int i = 0; i < 8; ++i) {
          delay(10);
          debounce &= (digitalRead(button2Pin) == HIGH) ? true : false;
        }
        if (debounce) {
          press2 = true;
          pressType = 0;
        }
      }
      break;
    case 1 | 2:
      //if (isReleased(&b1) || isReleased(&b2)) {
      // Register the double press when both buttons are released
      if (b1.cur == HIGH && b2.cur == HIGH) {
        press12 = true;
        pressType = 0;
      }
    break;  
    case 4:
      if (isReleased(&b3)) {
        // Debouncing
        bool debounce = true;
        for (int i = 0; i < 8; ++i) {
          delay(10);
          debounce &= (digitalRead(button3Pin) == HIGH) ? true : false;
        }
        if (debounce) {
          press3 = true;
          pressType = 0;
        }
      }
      break;
    default:
      pressType = 0;
  }

  // Button press debugging
  if (press1) 
    Serial.println("P1");
  if (press2) 
    Serial.println("P2");
  if (press12) 
    Serial.println("P12");
  if (press3) 
    Serial.println("P3");

  b1.prev = b1.cur;
  b2.prev = b2.cur;
  b3.prev = b3.cur;
}

void setup() {
  pinMode(contrastPin, OUTPUT);

  b1.cur  = HIGH;
  b1.prev = HIGH;
  b2.cur  = HIGH;
  b2.prev = HIGH;
  b3.cur  = HIGH;
  b3.prev = HIGH;

  initializePitchNames();
  analogWrite(contrastPin, 0);

  Serial.begin(9600);
  lcd.begin(16, 2);
  // Make sure the LCD is working
  lcd.print("/(;`^`)/");
  Serial.println("Setup complete.");

}

void loop() {
  checkButtons();

  switch (curScreen) {
    case 0: metronomeScreen(); break;
    case 1: tunerScreen(); break;
  }
  
  // Handle a next screen-press
  if (press12) {
    updateScreen = true;
    lcd.clear();

    curScreen = (curScreen + 1) % 2;
  }

  press1 = false;
  press2 = false;
  press12 = false;
  press3 = false;
}
