const int sensorPin = A5;
const float highTemp = 26.0;
float baselineTemp = 0.0;
int baselineCount = 0;

const int tempValsSize = 10;
float * tempVals = new float [tempValsSize];
float * baselineVals = new float[tempValsSize];

const int ledPinGreen = 11;
const int ledPinBlue = 10;
const int ledPinRed = 9;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < tempValsSize; ++i)
    tempVals[i] = baselineTemp;
  
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
}

void displayTemp(float temperature) {
  float tempSum = 0.0;
  for (int i = 0; i < tempValsSize - 1; ++i) {
    tempVals[i] = tempVals[i+1];
    tempSum += tempVals[i];
  }
  tempVals[tempValsSize - 1] = temperature;
  tempSum += temperature;

  float averageTemp = tempSum / tempValsSize;

  Serial.print(" Avg Temp: ");
  Serial.println(averageTemp);

  int blueVal = 255 * (highTemp - averageTemp) / (highTemp - baselineTemp);
  if (blueVal < 0)
    blueVal = 0;
  else if (blueVal > 255)
    blueVal = 255;
  int redVal = 255 * (averageTemp - baselineTemp) / (highTemp - baselineTemp);
  if (redVal < 0)
    redVal = 0;
  else if (redVal > 255)
    redVal = 255;
    
  analogWrite(ledPinBlue, blueVal);
  analogWrite(ledPinRed, redVal);
}

void loop() {
  int sensorVal = analogRead(sensorPin);
  float voltage = (sensorVal / 1024.0) * 5.0;
  float temperature = (voltage - 0.5) * 100;

  Serial.print("Sensor Val: ");
  Serial.print(sensorVal);
  Serial.print(" Voltage: ");
  Serial.print(voltage);
  Serial.print(" Temperature: ");
  Serial.print(temperature);

  if (baselineCount < tempValsSize) {
    baselineVals[baselineCount++] = temperature;
  } else if (baselineCount == tempValsSize) {
    float sum = 0.0;
    for (int i = 0; i < tempValsSize; ++i)
      sum += baselineVals[i];
    baselineTemp = sum / tempValsSize;
    baselineCount++;
  } else {
    displayTemp(temperature);
  }

}



