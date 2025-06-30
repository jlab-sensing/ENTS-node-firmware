/**
 * Example code to put the esp32 into a deep sleep mode.
 *
 * The start of the code waits 5 seconds before entering a deep sleep state so
 * that it can be verified that the esp32 is in fact going into deep sleep.
 *
 * @author John Madden
 * @date 2025-06-30
 */

#include <Arduino.h>
#include <ArduinoLog.h>

void setup() {
  // setup serial
  Serial.begin(115200);

  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  Log.noticeln("ents-node esp32 firmware, compiled at %s %s", __DATE__,
               __TIME__);
  Log.noticeln("Git SHA: %s", GIT_REV);


  // Enter deep sleep state
  Log.noticeln("Entering deep sleep mode in 5 seconds");

  delay(5000);

  Log.noticeln("Entering deep sleep mode!");

  esp_deep_sleep_start();
}

void loop() {}
