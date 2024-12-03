#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>
#include <FirebaseESP32.h>

#define SENSOR_PIN 33 // Define GPIO33 as the sensor pin
#define DEBOUNCE_DELAY 50 // Debounce time in milliseconds
#define SLEEP_TIMEOUT 180000 // time (ms) before entering deep sleep if nothing is detected
#define LOG_FILE "/log.json" // Path to the Log file
#define MAX_LOG_ENTRIES 100 // Maximum number of logs to keep in the file
#define FIREBASE_HOST "https://object-counter-db-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "TUgClLPtXF3vpOI6VfnExXLtkozFCNQwTYnouGve"

// put function declarations here:
//variables for debounce
unsigned long lastActivityTime = 0;
unsigned long lastDebounceTime = 0; // Last time the sensor state changed
int lastStableState = LOW; // Last stbale state of the sensor
int currentState = LOW; // Current state of the sensor
//Wifi credentials
// const char* ssid = "E308"; // Wi-fi SSID
// const char* password = "98806829"; // Wi-fi password
const char* ssid = "Dio Brando"; // Wi-fi SSID
const char* password = "mudamuda10"; // Wi-fi password
// NTP server and timezone settings
const char* ntpServer = "time.google.com"; //NTP server
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Initialize SPIFFS
void initializeSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    while (true);    
  }  
}

// Get the current timestamp as a string
String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "1970-01-01T00:00:00";  // Return a default value if time is invalid
  }
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buffer);
}

// Save a log entry to the JSON file
void logEvent(const String& timestamp, const String& event)
{
  // Skip if timestamp or event is invalid
  if (timestamp == "1970-01-01T00:00:00" || event.isEmpty()) {
    Serial.println("Invalid data, skipping log entry.");
    return;  // Skip logging if data is invalid
  }

  // Open or create the log file
  File file = SPIFFS.open(LOG_FILE, FILE_READ);
  DynamicJsonDocument doc(2048);

  if (file)
  {
    // Parse existing JSON
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
      Serial.println("Failed to parse existing JSON. Overwriting.");
      doc.clear();  // If JSON parsing fails, clear the document to start fresh
    }
  }

  // Ensure the log is an array
  if (!doc.is<JsonArray>())
  {
    doc.clear();  
    JsonArray logArray = doc.to<JsonArray>();  // Create an empty array
  }

  JsonArray logArray = doc.as<JsonArray>();

  // Add new log entry
  JsonObject logEntry = logArray.createNestedObject();
  logEntry["timestamp"] = timestamp;
  logEntry["event"] = event;

  // Limit Log size
  if (logArray.size() > MAX_LOG_ENTRIES)
  {
    logArray.remove(0);  // Remove the oldest log entry if it exceeds the max size
  }

  // Save updated JSON back to file
  file = SPIFFS.open(LOG_FILE, FILE_WRITE);
  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write log to file");
  }
  else
  {
    Serial.println("Log saved to SPIFFS");
  }
  file.close();

  // Upload to Firebase
  String logData;
  size_t len = measureJson(doc); // Measure the length of the serialized JSON
  char buffer[len + 1];          // Create a buffer of the appropriate size
  serializeJson(doc, buffer, sizeof(buffer)); // Serialize JSON into the buffer
  logData = String(buffer);      // Convert the buffer to an Arduino String

  // Check the serialized data to verify it's a valid JSON string
  Serial.println("Serialized Log Data: ");
  Serial.println(logData);  // Print out the JSON data being sent to Firebase

  if (!Firebase.RTDB.setString(&firebaseData, "/logs", logData))
  {
    Serial.println("Failed to upload log to Firebase:");
    Serial.println(firebaseData.errorReason());
  }
  else
  {
    Serial.println("Log uploaded to Firebase.");
  }
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");  
}

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print the local IP address

  // Initialize and configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  delay(5000);

  // Check if the time was successfully obtained
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    Serial.println("Time obtained successfully");
    printLocalTime();
  }

  // Initialize SPIFFS
  initializeSPIFFS();

  // Set Firebase configuration
  firebaseConfig.database_url = FIREBASE_HOST; // set the firebase URL
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Initialize Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth);

  // Deep sleep configuration
  pinMode(SENSOR_PIN, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); // Set GPIO33 as wake-up source on HIGH signal

  Serial.println("Sensor monitoring with debounce...");
}

void loop() {
  int reading = digitalRead(SENSOR_PIN); // Read the current state of the sensor
  
  // Check if the sensor state has changed
  if (reading != lastStableState)
  {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // If the state has been stable for the debounce delay
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) 
  {
    // Only update the current state if it has truly changed
    if (reading != currentState)
    {
      currentState = reading;

      // Log the stable state
      String timestamp = getTimestamp();
      if (currentState == HIGH)
      {
        Serial.println("Object detected!");
        logEvent(timestamp, "Object detected");
        lastActivityTime = millis(); // Reset the inactivity timer
      }
      else
      {
        Serial.println("No object detected.");
        logEvent(timestamp, "No object detected");
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
