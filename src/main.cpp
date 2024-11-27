#include <Arduino.h>


#define SENSOR_PIN 33 // Define GPIO33 as the sensor pin
#define DEBOUNCE_DELAY 50 // Debounce time in milliseconds
#define SLEEP_TIMEOUT 10000 // time (ms) before entering deep sleep if nothing is detected
// 10 000 milliseconds is 10 seconds

// put function declarations here:
// int myFunction(int, int);
unsigned long lastActivityTime = 0;
unsigned long lastDebounceTime = 0; // Last time the sensor state changed
int lastStableState = LOW; // Last stbale state of the sensor
int currentState = LOW; // Current state of the sensor


void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); // Set GPIO33 as wake-up source on HIGH signal
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
       lastActivityTime = millis(); // Reset the inactivity timer
      }
      else
      {
        Serial.println("No object detected.");
      }
      
    }
  }

  lastStableState = reading;

  // Check for inactivity to enter deep sleep
  if (millis() - lastActivityTime > SLEEP_TIMEOUT)
  {
    Serial.println("Entering deep sleep...");
    delay(100); // Allow Serial message to be sent before sleep
    esp_deep_sleep_start(); // Enter deep sleep
  }
  
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }