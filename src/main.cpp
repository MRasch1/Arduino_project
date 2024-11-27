#include <Arduino.h>


#define SENSOR_PIN 33 // Define GPIO33 as the sensor pin
#define DEBOUNCE_DELAY 50 // Debounce time in milliseconds


// put function declarations here:
// int myFunction(int, int);
unsigned long lastDebounceTime = 0; // Last time the sensor state changed
int lastStableState = LOW; // Last stbale state of the sensor
int currentState; // Current state of the sensor


void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  Serial.println("Sensor monitoring with debounce...");
}

void loop() {
  // put your main code here, to run repeatedly:
  int reading = digitalRead(SENSOR_PIN); // Read the current state of the sensor
  
  // Check if the sensor state has changed
  if (reading != lastStableState)
  {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // If the state has been stable for the debounce delay
  if((millis() - lastDebounceTime) > DEBOUNCE_DELAY) 
  {
    //only update the current state if it has truly changed
    if(reading != currentState)
    {
      currentState = reading;

      // print the stable state
      if (currentState == HIGH)
      {
       Serial.println("Object detected!");
      }
      else
      {
        Serial.println("No object detected.");
      }
      
    }
  }

  lastStableState = reading;
  
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }