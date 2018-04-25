#include <LiquidCrystal.h>
#include <stdbool.h>
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

const int contrastPin = 9;
const int button1Pin = 12;
const int button2Pin = 13;
const int button3Pin = 11;
const int potPin = A5;
const int buzzerPin = 8;

unsigned char num1, num2;
int byteCounter = 0;

int curScreen = 0;
int contrast = 0;

// Easily keep track of button states
typedef struct {
  int cur;
  int prev;
} ButtonState;

ButtonState b1, b2, b3;

bool press1 = false, press2 = false, press12 = false, press3 = false;

// Store the type of press as a binary number where each bit
// represents a button press. This allows for multi-button presses
// 0 - none, 1 - b1, 2 - b2, 3 - b1 & b2, 4 - b3
int pressType = 0;

// Keep track of whether the screen needs updating
bool updateScreen = true;

void splashScreen() {
  if (updateScreen) {
    lcd.setCursor(0,0);
    lcd.write("Welcome to the");
    lcd.setCursor(0,1);
    lcd.print("Splash Screen!");
    updateScreen = false;
  }
}

// Print 8 bits
void printByte(unsigned char * n) {
  for (int i = 0; i < 8; ++i) {
    lcd.print((*n >> (7-i)) & 1);
  }
}

void byteScreen(unsigned char * n, int byteNum) {
  if (press1) {
    // Change the correct bit based on where the counter is
    *n ^= 0x80 >> byteCounter;
    updateScreen = true;
  } else if (press2) {
    //byteCounter = (byteCounter + 1) % 8;
    byteCounter = byteCounter + 1;
    // When the last byte is reached, simulate a next-screen press
    if (byteCounter == 8) {
      byteCounter = 0;
      press12 = true;
    }
  } else if (press12) {
    byteCounter = 0;
  }
  
  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Byte ");
    lcd.print(byteNum);
    lcd.print(":");
    //lcd.print(*n);
    lcd.setCursor(0,1);
    printByte(n);
    updateScreen = false;
  }
}

void solutionScreen() {
  if (updateScreen) {
    lcd.setCursor(0,0);
    lcd.print("Sum:");
    unsigned char sum;
    sum = num1 + num2;
    //lcd.print(sum);
    lcd.setCursor(0,1);
    // Add the carry bit if the number overflows 1 byte
    if (num1 + num2 > 0xff)
      lcd.print(1);
    else
      lcd.print(0);
    printByte(&sum);
    updateScreen = false;
  }
  //printf("solution: %d\n", num1 + num2);
}

void contrastScreen() {
  if (press1) {
    contrast += 10;
    updateScreen = true;
  } else if (press2) {
    contrast -= 10;
    updateScreen = true;
  }
  // Ensure the contrast stays within range
  contrast = (contrast < 0) ? 0 : contrast;
  contrast = (contrast > 100) ? 100 : contrast;

  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Contrast");
    lcd.setCursor(0,1);
    lcd.print("Left- Right+");
    /*
    lcd.setCursor(0,0);
    lcd.print("Contrast:");
    lcd.setCursor(0,1);
    lcd.print(contrast);
    */
    updateScreen = false;
  }
}

bool tunerOn = false;
double tunerBase = 440.0;
int tunerPitch = 0;

const char * pitchNames[12];

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
  }

  double oldBase = tunerBase;
  int potPitch = 6*analogRead(potPin)/1024;
  switch (potPitch) {
    case 0: tunerBase = 415.0; break;
    case 1: tunerBase = 430.0; break;
    case 2: tunerBase = 440.0; break;
    case 3: tunerBase = 441.0; break;
    case 4: tunerBase = 442.0; break;
    case 5: tunerBase = 443.0; break;
    default: tunerBase = 443.0;
  }
  if (oldBase != tunerBase)
    updateScreen = true;
  

  if (tunerOn) {
    tone(buzzerPin, tunerBase * pow(2.0, tunerPitch/12.0));
  } else {
    noTone(buzzerPin);
  }

  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Base: ");
    lcd.print(tunerBase);
    lcd.setCursor(0,1);
    lcd.print("Note: ");
    lcd.print(pitchNames[(1200+tunerPitch) % 12]);
    lcd.print((57+tunerPitch)/12);
    updateScreen = false;
  }
}

bool metronomeOn = false;
int metronomeTempo = 100;
int metronomeBeats = 1;
int beatNumber = 0;
bool playBeat = false;
long long currentTime;
long long oldTime;

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
  }

  double oldBeats = metronomeBeats;
  int potPitch = 4*analogRead(potPin)/1024;
  if (oldBeats != metronomeBeats)
    updateScreen = true;
  

  if (metronomeOn) {
    if (currentTime - oldTime > (36000.0/metronomeTempo)) {
      playBeat = true;
    }
  } else {
  }

  if (playBeat) {
    tone(buzzerPin, 440, 50);
    playBeat = false;
    oldTime = currentTime;
  }

  if (updateScreen) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Metronome");
    lcd.setCursor(0,1);
    lcd.print("Note: ");
    updateScreen = false;
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

  Serial.begin(9600);
  lcd.begin(16, 2);
  // Make sure the LCD is working
  lcd.print("/(;`^`)/");
  Serial.println("Setup complete.");

}

void loop() {
  analogWrite(contrastPin, (contrast/100.0)*255);
  //analogWrite(contrastPin, 0);

  checkButtons();

/*
  switch (curScreen) {
    case 0: splashScreen(); break;
    case 1: byteScreen(&num1, 1); break;
    case 2: byteScreen(&num2, 2); break;
    case 3: solutionScreen(); break;
    case 4: contrastScreen(); break;
  }
*/
  switch (curScreen) {
    case 0: metronomeScreen(); break;
    case 1: tunerScreen(); break;
  }

  
  // Handle a next screen-press
  if (press12) {
    updateScreen = true;
    lcd.clear();

    // Ensure that the splash screen is only shown at the beginning
    if (curScreen == 4)
      curScreen = 1;
    else
      ++curScreen;
  }

  //printf("At screen: %d\n", curScreen);
  
  //printf("contrast: %d\n", contrast);
  //printf("bytes: %d %d\n", num1, num2);
  
  //printf("press: %d %d %d\n", press1, press2, press12);
  
  //Serial.println(analogRead(potPin));

  press1 = false;
  press2 = false;
  press12 = false;
  press3 = false;
}
