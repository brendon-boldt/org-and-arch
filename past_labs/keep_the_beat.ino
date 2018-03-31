#include <Servo.h>

Servo myServo;
const int servoPin = 3;
const int switchPin = 2;
const int buzzerPin = 11;
unsigned long oldTime = 0;
unsigned long currentTime = 0;
unsigned long lightInterval = 1000;
int prevSwitchState = 0;
int switchState = 0;
bool isPlaying = false;

const int Q = 500;
const float
R = 0.0 ,
pA4  = 440 ,
pAis4 = 466.16  ,
pB4 = 493.88  ,
pC5 = 523.25  ,
pCis5 = 554.37  ,
pD5 = 587.33  ,
pDis5 = 622.25  ,
pE5 = 659.25  ,
pF5 = 698.46  ,
pFis5 = 739.99  ,
pG5 = 783.99  ,
pGis5 = 830.61  ,
pA5 = 880 ,
pAis5 = 932.328 ,
pB5 = 987.767 ,
pC6 = 1046.502  ,
pCis6 = 1108.731  ,
pD6 = 1174.659  ,
pDis6 = 1244.508  ,
pE6 = 1318.51 ,
pF6 = 1396.913;
  

struct Piece {
  int length;
  float * pitches;
  int * delays;
};

Piece paganini;
Piece danse;

struct Led {
  int pin;
  int state;
};

const int lightArraySize = 4;
Led * lightArray = new Led[lightArraySize]; 

Led * Led_new(int pin, int state) {
  Led * p = (Led*) malloc(sizeof(Led));
  p->pin = pin;
  p->state = state;
  pinMode(pin, OUTPUT);
  return p;
}

float paganiniPitches[] = {pA5,   R, pA5,   R, pA5, pC6, pB5, pA5, pE6,   R, pE5,   R, pE5, pGis5, pFis5, pE5, pA5,   R, pA5,   R, pA5, pC6, pB5, pA5, pE6, pE5}; 
int paganiniDelays[] = {Q/2, Q/4, Q/8, Q/8, Q/4, Q/4, Q/4, Q/4, Q/2, Q/4, Q/8, Q/8, Q/4,   Q/4,   Q/4, Q/4, Q/2, Q/4, Q/8, Q/8, Q/4, Q/4, Q/4, Q/4,   Q, Q/2}; 

float dansePitches[] = {pG5, pAis5, pG5, pA5, pAis5, pG5, pAis5, pG5, pA5, pAis5, pA5, pG5, pAis5, pG5, pA5, pAis5, pG5, pAis5, pG5, pA5, pGis5};
int danseDelays [] =  {Q/2,   Q/2, Q/2, Q/2,     Q, Q/2,   Q/2, Q/2, Q/2,   Q/2, Q/2, Q/2,   Q/2, Q/2, Q/2,     Q, Q/2,   Q/2, Q/2, Q/2,     Q};
  
void setup() {
  Serial.begin(9600);
  pinMode(switchPin, INPUT);
  myServo.attach(servoPin);
  
  lightArray[3] = *Led_new(5, LOW);
  lightArray[2] = *Led_new(8, LOW);
  lightArray[1] = *Led_new(10, LOW);
  lightArray[0] = *Led_new(12, LOW);

  paganini.pitches = paganiniPitches;
  paganini.delays  = paganiniDelays;
  paganini.length  = 26;

  danse.pitches = dansePitches;
  danse.delays  = danseDelays;
  danse.length  = 21;
}

void setLights() {
  for (int i = 0; i < lightArraySize; i++) {
    digitalWrite(lightArray[i].pin, lightArray[i].state);
  }

}

const int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

void noteLigths(float note) {
  for (int i = 0; i < lightArraySize; ++i) {
    lightArray[i].state = (((int) note % primes[i]) % 2) ? HIGH : LOW;
  }
  setLights();
}

void play(Piece * piece) {
  for (int i = 0; i < piece->length; ++i) {
    noteLigths(piece->pitches[i]);
    //Serial.println(piece->pitches[i]);
    /*
     */
    if (piece->pitches[i] > 20.0)
      tone(buzzerPin, piece->pitches[i]);
    else
      noTone(buzzerPin);
    delay(piece->delays[i]);
  }
  noTone(buzzerPin);
}

void wiggle() {
  for (int i = 0; i < 2; ++i) {
    for (int angle = 0; angle < 179; angle += 178) {
      myServo.write(angle);
      delay(500);
    }
  }
  myServo.write(0);

}

void loop() {
  switchState = digitalRead(switchPin);
  currentTime = millis() - oldTime;

  if (switchState != prevSwitchState) {
    oldTime = millis();
  }
  prevSwitchState = switchState;

  int numLit = currentTime / lightInterval;
  if (numLit > lightArraySize) {
    isPlaying = true;
    play(&paganini);
    isPlaying = false;
    wiggle();
    oldTime = millis();
  }
  for (int i = 0; i < lightArraySize; ++i)
    lightArray[i].state = (i < numLit) ? HIGH : LOW;

  setLights();
}




