#include "webserver.hpp"

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoLog.h>


WebServer server(80);

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
  server.send(200, "text/plain", "");
}

void HandleClose() {
  Log.noticeln("Closing valve");
  server.send(200, "text/plain", "");
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

  Log.noticeln("Opening valve for %d seconds", time);
  server.send(200, "text/plain", "");
}

void HandleState() {
  Log.noticeln("Getting current state");
  server.send(200, "text/plain", "open");
}

