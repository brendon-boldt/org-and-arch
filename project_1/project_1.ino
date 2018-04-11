#include <LiquidCrystal.h>
#include <stdbool.h>
LiquidCrystal lcd(8, 9, 2, 3, 4, 5);

const int contrastPin = 11;
const int button1Pin = 12;
const int button2Pin = 13;

unsigned char num1, num2;
int byteCounter = 0;

int curScreen = 0;
int contrast = 0;

typedef struct {
  int cur;
  int prev;
} ButtonState;

ButtonState b1, b2;

bool press1 = false, press2 = false, press12 = false;

// 0 - none, 1 - b1, 2 - b2, 3 - b1 & b2
int pressType = 0;

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

void printByte(unsigned char * n) {
  for (int i = 0; i < 8; ++i) {
    lcd.print((*n >> (7-i)) & 1);
  }
}

void byteScreen(unsigned char * n, int byteNum) {
  if (press1) {
    *n ^= 0x80 >> byteCounter;
    updateScreen = true;
  } else if (press2) {
    //byteCounter = (byteCounter + 1) % 8;
    byteCounter = byteCounter + 1;
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

bool isReleased(ButtonState * b) {
  return b->cur == HIGH && b->prev == LOW;
}

void checkButtons() {
  b1.cur = digitalRead(button1Pin);
  b2.cur = digitalRead(button2Pin);

  if (b1.cur == LOW)
    pressType |= 1;
  if (b2.cur == LOW)
    pressType |= 2;

  switch (pressType) {
    case 0:
      break;
    case 1:
      if (isReleased(&b1)) {
        press1 = true;
        pressType = 0;
      }
      break;
    case 2:
      if (isReleased(&b2)) {
        press2 = true;
        pressType = 0;
      }
      break;
    case 1 | 2:
      //if (isReleased(&b1) || isReleased(&b2)) {
      if (b1.cur == HIGH && b2.cur == HIGH) {
        press12 = true;
        pressType = 0;
      }
  }

  if (press1) 
    Serial.println("P1");
  if (press2) 
    Serial.println("P2");
  if (press12) 
    Serial.println("P12");

  b1.prev = b1.cur;
  b2.prev = b2.cur;
}




void setup() {
  pinMode(contrastPin, OUTPUT);

  b1.cur  = HIGH;
  b1.prev = HIGH;
  b2.cur  = HIGH;
  b2.prev = HIGH;

  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("/(;`^`)/");
  Serial.println("Setup complete.");

}

void loop() {
  analogWrite(contrastPin, (contrast/100.0)*255);
  Serial.println(contrast);
  //analogWrite(contrastPin, 0);

	checkButtons();

	switch (curScreen) {
		case 0: splashScreen(); break;
		case 1: byteScreen(&num1, 1); break;
		case 2: byteScreen(&num2, 2); break;
		case 3: solutionScreen(); break;
		case 4: contrastScreen(); break;
	}

	
	if (press12) {
    updateScreen = true;
    lcd.clear();
    if (curScreen == 4)
      curScreen = 1;
		else
      ++curScreen;
  }

	//printf("At screen: %d\n", curScreen);
	
	//printf("contrast: %d\n", contrast);
	//printf("bytes: %d %d\n", num1, num2);
	
	//printf("press: %d %d %d\n", press1, press2, press12);

	press1 = false;
	press2 = false;
	press12 = false;
}
