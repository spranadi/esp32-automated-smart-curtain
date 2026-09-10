#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "time.h"

#include "config.h"
#include "sensitive.h"
#include "MotorController.h"
#include "WebPage.h"

// System instances
MotorController motor(PIN_STEP, PIN_DIR, PIN_ENABLE);
WebServer server(80);
Preferences preferences;

// Logging & State
String wirelessLogBuffer = "";
int targetOpenHour     = 7;
int targetOpenMinute   = 0;
int targetCloseHour    = 20;
int targetCloseMinute  = 0;

// Debounce Tracking (To avoid false triggers)
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// Forward Declarations (To accommodate functions that are defined after their usage)
void logMessage(String msg);
void serviceNetwork();
void handleRoot();
void handleSave();
void handleLogs();

//Caps message size at 3000 characters, trimming the oldest logs if necessary
void logMessage(String msg) {
  Serial.println(msg);
  wirelessLogBuffer += msg + "\n";
  if (wirelessLogBuffer.length() > 3000) {
    wirelessLogBuffer = wirelessLogBuffer.substring(wirelessLogBuffer.length() - 2000);
  }
}

//Allows web server usage during motor movement
void serviceNetwork() {
  server.handleClient();
}

void setup() {
  Serial.begin(115200); // Default baud rate for ESP32 Serial Monitor
  delay(500);

  motor.begin();
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  logMessage("\n=========================================");
  logMessage("  Smart Curtain Controller (High-Torque)");
  logMessage("  Initial State: CLOSED (0 steps)");
  logMessage("=========================================");

  // 1. Connect Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  logMessage("\nConnected! IP Address: " + WiFi.localIP().toString());

  // 2. Synchronize Clock via NTP
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // 3. Load Saved Schedule from NVS
  preferences.begin("curtain", true);
  targetOpenHour    = preferences.getInt("openH", 7);
  targetOpenMinute  = preferences.getInt("openM", 0);
  targetCloseHour   = preferences.getInt("closeH", 20);
  targetCloseMinute = preferences.getInt("closeM", 0);
  preferences.end();

  char scheduleMsg[120];
  snprintf(scheduleMsg, sizeof(scheduleMsg), 
           "Loaded Schedule -> Open: %02d:%02d | Close: %02d:%02d", 
           targetOpenHour, targetOpenMinute, targetCloseHour, targetCloseMinute);
  logMessage(scheduleMsg);

  // 4. Register HTTP Web Server Routes
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/logs", handleLogs);
  server.begin();
  logMessage("HTTP Web Server & Live Console started.");
}

void loop() {
  server.handleClient();

  // Scheduled Automation Check
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    int currentHour   = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;

    // Print clock and curtain state every 10 seconds
    static unsigned long lastClockPrint = 0;
    if (millis() - lastClockPrint >= 10000) {
      lastClockPrint = millis();
      char timeMsg[80];
      snprintf(timeMsg, sizeof(timeMsg), "Clock: %02d:%02d:%02d | Curtain: %s", 
               currentHour, currentMinute, timeinfo.tm_sec, 
               (motor.getState() == OPEN) ? "OPEN" : "CLOSED");
      logMessage(timeMsg);
    }

    //Trigger scheduled open/close actions at the exact minute and second 0
    if (currentHour == targetOpenHour && currentMinute == targetOpenMinute && 
        timeinfo.tm_sec == 0 && motor.getState() == CLOSED) {
      logMessage(">> Scheduled Open Triggered!");
      motor.moveTo(POSITION_OPEN, OPEN, serviceNetwork);
    }

    if (currentHour == targetCloseHour && currentMinute == targetCloseMinute && 
        timeinfo.tm_sec == 0 && motor.getState() == OPEN) {
      logMessage(">> Scheduled Close Triggered!");
      motor.moveTo(POSITION_CLOSED, CLOSED, serviceNetwork);
    }
  }

  // Manual Pushbutton Toggle Logic
  int reading = digitalRead(PIN_BUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    static int buttonState = HIGH;
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        if (motor.getState() == CLOSED) {
          logMessage("Manual Trigger: Opening...");
          motor.moveTo(POSITION_OPEN, OPEN, serviceNetwork);
        } else {
          logMessage("Manual Trigger: Closing...");
          motor.moveTo(POSITION_CLOSED, CLOSED, serviceNetwork);
        }
      }
    }
  }
  lastButtonState = reading;
}

//Sends dashboard webpage to browser with live curtain feed
void handleRoot() {
  String html = getIndexPage(motor.getState() == OPEN, targetOpenHour, targetOpenMinute, targetCloseHour, targetCloseMinute);
  server.send(200, "text/html", html);
}

//Sends raw terminal logs to browser for live monitoring
void handleLogs() {
  server.send(200, "text/plain", wirelessLogBuffer);
}

//Handles saving the new open/close schedule and reloads page
void handleSave() {
  if (server.hasArg("open") && server.hasArg("close")) {
    String openVal  = server.arg("open");
    String closeVal = server.arg("close");

    targetOpenHour    = openVal.substring(0, 2).toInt();
    targetOpenMinute  = openVal.substring(3, 5).toInt();
    targetCloseHour   = closeVal.substring(0, 2).toInt();
    targetCloseMinute = closeVal.substring(3, 5).toInt();

    preferences.begin("curtain", false);
    preferences.putInt("openH", targetOpenHour);
    preferences.putInt("openM", targetOpenMinute);
    preferences.putInt("closeH", targetCloseHour);
    preferences.putInt("closeM", targetCloseMinute);
    preferences.end();

    logMessage("Schedule Updated via Web -> Open: " + openVal + " | Close: " + closeVal);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated");
}