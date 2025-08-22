#include "webserver.hpp"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <WebServer.h>

WebServer server(80);

// Global variable to track solenoid state
IrrigationCommand_State currentSolenoidState = IrrigationCommand_State_CLOSE;

// Timer variables for timed operation
unsigned long timedStartTime = 0;
unsigned long timedDuration = 0;
bool timedOperation = false;

// Implement the getter function
IrrigationCommand_State GetSolenoidState() {
    return currentSolenoidState;
}

// Implement the setter function  
void SetSolenoidState(IrrigationCommand_State newState) {
    currentSolenoidState = newState;
    Log.noticeln("Solenoid state changed to: %d", newState);
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
 * @brief Handles requests to /state
 *
 * Gets the current state of the valve.
 */
void HandleState();

void HandleClient() { 
  server.handleClient(); 
  UpdateTimedOperation(); // Check for timed operations
}

void SetupServer() {
  server.on("/open", HTTP_POST, HandleOpen);
  server.on("/close", HTTP_POST, HandleClose);
  server.on("/timed", HTTP_POST, HandleTimed);
  server.on("/state", HTTP_GET, HandleState);
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

void HandleState() {
  
  IrrigationCommand_State currentState = GetSolenoidState();
  String stateStr = (currentState == IrrigationCommand_State_OPEN) ? "open" : "closed";
  Log.noticeln("Current state: %s (%d)", stateStr.c_str(), currentState);
  server.send(200, "text/plain", stateStr);
}
