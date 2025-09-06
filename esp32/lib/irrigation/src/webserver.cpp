#include "webserver.hpp"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <WebServer.h>

// External declarations for irrigation functions
extern void EnableAutoIrrigation(float min_thresh, float max_thresh);
extern void DisableAutoIrrigation();
extern bool IsAutoIrrigationEnabled();
extern float GetMinThreshold();
extern float GetMaxThreshold();
extern unsigned long GetCheckInterval();
extern float GetCurrentMoistureFromCache();
extern void SetCheckInterval(unsigned long interval_ms);
extern void CheckIrrigationConditions();

WebServer server(80);

// Global variable to track solenoid state
IrrigationCommand_State currentSolenoidState = IrrigationCommand_State_CLOSE;

// Timer variables for timed operation
unsigned long timedStartTime = 0;
unsigned long timedDuration = 0;
bool timedOperation = false;

//Thresholds for auto irrigation
extern float moisture_min_threshold;
extern float moisture_max_threshold;
extern unsigned long check_interval;

// Current moisture reading from irrigation module
float current_moisture = -1.0;

extern bool auto_irrigation_enabled;
unsigned long last_moisture_check = 0;
const unsigned long MOISTURE_CHECK_INTERVAL = 300000; // 5 minutes

// Implement the getter function
IrrigationCommand_State GetSolenoidState() {
    return currentSolenoidState;
}

// Implement the setter function  
void SetSolenoidState(IrrigationCommand_State newState) {
    currentSolenoidState = newState;
    Log.noticeln("Solenoid state changed to: %d", newState);
}

// Getter and setter for moisture
float GetCurrentMoisture() {
  return GetCurrentMoistureFromCache();
}

void SetCurrentMoisture(float moisture) {
  current_moisture = moisture;
  Log.noticeln("Received moisture reading: %.1f%%", moisture);
}

// Function to update timed operations
void UpdateTimedOperation() {
  if (timedOperation && millis() - timedStartTime >= timedDuration) {
      Log.noticeln("Timed operation completed, closing valve");
      SetSolenoidState(IrrigationCommand_State_CLOSE);
      timedOperation = false;
  }
}

/**
 * @brief Handles POST requests to /on
 *
 * Forces the valve into the open state.
 */
void HandleOpen();

/**
 * @brief Handles POST requests to /off
 *
 * Forces the valve into the closd state.
 */
void HandleClose();

/**
 * @brief Handles POST requests to /timed
 *
 * Opens the valve for a specified time. If the valve is already open, it will
 * closed afer the specified amount of time.
 */
void HandleTimed();

/**
 * @brief Handles auto irrigation configuration
 */
void HandleAutoIrrigationSetup();

/**
 * @brief Handles auto irrigation toggle
 */
void HandleAutoToggle();

/**
 * @brief Handles requests to /state
 *
 * Gets the current status of the auto irrigation.
 */
void HandleStatus();

/**
 * @brief Handles requests to /state
 *
 * Gets the current state of the valve.
 */
void HandleState();

void HandleMoisture();

void HandleClient() { 
  server.handleClient(); 
  UpdateTimedOperation(); // Check for timed operations
}

void SetupServer() {
  server.on("/open", HTTP_POST, HandleOpen);
  server.on("/close", HTTP_POST, HandleClose);
  server.on("/timed", HTTP_POST, HandleTimed);
  server.on("/state", HTTP_GET, HandleState);
  server.on("/auto", HTTP_POST, HandleAutoToggle);
  server.on("/irrigation_setup", HTTP_POST, HandleAutoIrrigationSetup);
  server.on("/status", HTTP_GET, HandleStatus);
  server.on("/moisture", HTTP_POST, HandleMoisture);
  server.begin();
}

void HandleOpen() {
  Log.noticeln("Opening valve");
  SetSolenoidState(IrrigationCommand_State_OPEN);
  timedOperation = false; // Cancel any timed operation
  server.send(200, "text/plain", "Opening valve");
}

void HandleClose() {
  Log.noticeln("Closing valve");
  SetSolenoidState(IrrigationCommand_State_CLOSE);
  timedOperation = false; // Cancel any timed operation
  server.send(200, "text/plain", "Closing valve");
}

void HandleTimed() {
  int time = 0;

  if (server.hasArg("time")) {
    String time_str = server.arg("time");
    time = time_str.toInt();
  } else {
    Log.errorln("Timed opening valve: no time specified");
    server.send(400, "text/plain", "No time specified");
    return;
  }

  if (time <= 0) {
    Log.errorln("Invalid time specified: %d", time);
    server.send(400, "text/plain", "Invalid time specified");
    return;
  }

  Log.noticeln("Opening valve for %d seconds", time);
  
  //Timed sequence
  HandleOpen();
  timedStartTime = millis();
  timedDuration = time * 1000; // Convert to milliseconds
  timedOperation = true;

  server.send(200, "text/plain", "Valve opened for set duration");
}

void HandleAutoIrrigationSetup() {
  if (server.hasArg("min") && server.hasArg("max")) {
      float min_thresh = server.arg("min").toFloat();
      float max_thresh = server.arg("max").toFloat();
      
      EnableAutoIrrigation(min_thresh, max_thresh);
      
      if (server.hasArg("interval")) {
          SetCheckInterval(server.arg("interval").toInt() * 60000);
      }
      
      Log.noticeln("Auto irrigation configured: Min=%.1f%%, Max=%.1f%%, Interval=%lu min", 
                  min_thresh, max_thresh, GetCheckInterval() / 60000);
      server.send(200, "text/plain", "Auto irrigation configured");
  } else {
      Log.errorln("Auto irrigation setup missing parameters");
      server.send(400, "text/plain", "Missing min/max parameters");
  }
}

void HandleAutoToggle() {
  if (server.hasArg("enable")) {
      bool enable = server.arg("enable") == "true";
      if (enable) {
          // Enable with current thresholds - let CheckAutoIrrigation handle the state
          EnableAutoIrrigation(GetMinThreshold(), GetMaxThreshold());
      } else {
          DisableAutoIrrigation();
          SetSolenoidState(IrrigationCommand_State_CLOSE);  // Still close when disabling
      }
      Log.noticeln("Auto irrigation %s", enable ? "enabled" : "disabled");
      server.send(200, "text/plain", enable ? "Auto enabled" : "Auto disabled");
  } else {
      server.send(400, "text/plain", "Missing enable parameter");
  }
}

void HandleStatus() {
  // Get current moisture reading from cache
  float current_moisture_value = GetCurrentMoisture();
  
  String json = "{";
  json += "\"solenoid_state\":\"" + String(GetSolenoidState() == IrrigationCommand_State_OPEN ? "open" : "closed") + "\",";
  json += "\"auto_irrigation_enabled\":" + String(IsAutoIrrigationEnabled() ? "true" : "false") + ",";
  json += "\"min_threshold\":" + String(GetMinThreshold(), 2) + ",";
  json += "\"max_threshold\":" + String(GetMaxThreshold(), 2) + ",";
  json += "\"check_interval_seconds\":" + String(GetCheckInterval() / 1000) + ","; // Convert ms to seconds
  json += "\"current_moisture\":" + String(current_moisture_value, 2);
  json += "}";
  
  server.send(200, "application/json", json);
}

void HandleState() {
  IrrigationCommand_State currentState = GetSolenoidState();
  String stateStr = (currentState == IrrigationCommand_State_OPEN) ? "open" : "closed";
  Log.noticeln("Current state: %s (%d)", stateStr.c_str(), currentState);
  server.send(200, "text/plain", stateStr);
}

void HandleMoisture() {
  if (server.hasArg("moisture")) {
    float moisture = server.arg("moisture").toFloat();
    SetCurrentMoisture(moisture);
    
    // Trigger auto irrigation check when new moisture data arrives
    if (IsAutoIrrigationEnabled()) {
      Log.noticeln("New moisture reading received, triggering irrigation check");
      CheckIrrigationConditions();
    }
    
    server.send(200, "text/plain", "Moisture received");
  }
  else {
    Log.errorln("Moisture endpoint missing parameter");
    server.send(400, "text/plain", "Missing moisture parameter");
  }
}