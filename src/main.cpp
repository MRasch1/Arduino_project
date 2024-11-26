#include <Arduino.h>

// put function declarations here:
// int myFunction(int, int);

#define SENSOR_PIN 33 // Define GPIO33 as the sensor pin

void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensorValue = digitalRead(SENSOR_PIN);
  if (sensorValue == HIGH)
  {
    Serial.println("Object detected!");
  }
  else
  {
    Serial.println("No object detected.");
  }
  delay(100);
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }