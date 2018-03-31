#include <stdio.h>
#include <stdbool.h>

// Debugging code
const int LOW = 0;
const int HIGH = 1;
int _button1;
int _button2;
//

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

void setup() {
	_button1 = LOW;
	_button2 = LOW;

	b1.cur  = LOW;
	b1.prev = LOW;
	b2.cur  = LOW;
	b2.prev = LOW;
}

void splashScreen() {
}

void byteScreen(unsigned char * byte) {
	if (press1) {
		*byte ^= 1 << byteCounter;
	} else if (press2) {
		byteCounter = (byteCounter + 1) % 8;
	} else if (press12) {
		byteCounter = 0;
	}
}

void solutionScreen() {
	printf("solution: %d\n", num1 + num2);
}

void contrastScreen() {
	if (press1) 
		contrast -= 10;
	else if (press2) 
		contrast += 10;
	contrast = (contrast < 0) ? 0 : contrast;
	contrast = (contrast > 100) ? 100 : contrast;
}

bool isReleased(ButtonState * b) {
	return b->cur == LOW && b->prev == HIGH;
}

void checkButtons() {
	b1.cur = _button1;
	b2.cur = _button2;

	if (b1.cur == HIGH)
		pressType |= 1;
	if (b2.cur == HIGH)
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
			if (isReleased(&b1) || isReleased(&b2)) {
				press12 = true;
				pressType = 0;
			}
	}

	b1.prev = b1.cur;
	b2.prev = b2.cur;
}

void loop() {

	checkButtons();
	
	if (press12)
		curScreen = (curScreen + 1) % 5;

	switch (curScreen) {
		case 0: splashScreen(); break;
		case 1: byteScreen(&num1); break;
		case 2: byteScreen(&num2); break;
		case 3: solutionScreen(); break;
		case 4: contrastScreen(); break;
	}

	//printf("At screen: %d\n", curScreen);
	
	//printf("contrast: %d\n", contrast);
	//printf("bytes: %d %d\n", num1, num2);
	
	//printf("press: %d %d %d\n", press1, press2, press12);

	press1 = false;
	press2 = false;
	press12 = false;
}

int main() {
	setup();
	press12 = true;
	loop();
	press12 = true;
	loop();
	press12 = true;
	loop();
	press12 = true;
	loop();
	press2 = true;
	loop();
	press2 = true;
	loop();
	press2 = true;
	loop();
	return 0;
}
