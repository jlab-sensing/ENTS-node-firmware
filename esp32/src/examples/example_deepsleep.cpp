/**
 * Example code for wakeup from deep sleep.
 *
 * Load example code on the stm32 that triggers the wakeup pin every 5 seoncds
 * and observe the boot count increasing.
 *
 * Some of this code is taken from esp32duino eamples.
 * https://docs.espressif.com/projects/arduino-esp32/en/latest/api/deepsleep.html
 *
 * @author John Madden
 * @date 2025-11-04
 */


#include <Arduino.h>
#include <ArduinoLog.h>

#include "driver/gpio.h"
#include "esp_sleep.h"



/** Number of times the esp32 has been booted */
RTC_DATA_ATTR int bootCount = 0;

/* Method to print the reason by which ESP32 has been awaken from sleep
 */
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    case ESP_SLEEP_WAKEUP_GPIO:              Serial.println("Wakeup caused by GPIO (light sleep only on ESP32, S2 and S3)"); break;
    case ESP_SLEEP_WAKEUP_UART:              Serial.println("Wakeup caused by UART0 (light sleep only)"); break;
    case ESP_SLEEP_WAKEUP_WIFI:              Serial.println("Wakeup caused by WIFI (light sleep only)"); break;
    case ESP_SLEEP_WAKEUP_COCPU:             Serial.println("Wakeup caused by COCPU int"); break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:   Serial.println("Wakeup caused by COCPU crash"); break;
    case ESP_SLEEP_WAKEUP_BT:                Serial.println("Wakeup caused by BT (light sleep only)"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void setup() {
  ++bootCount;

  // setup serial
  Serial.begin(115200);


  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  Log.noticeln("ents-node esp32 firmware, compiled at %s %s", __DATE__,
               __TIME__);
  Log.noticeln("Git SHA: %s", GIT_REV);
  Log.noticeln("Boot number: %d", bootCount);

 
  print_wakeup_reason();

  const gpio_num_t wakeup_pin = GPIO_NUM_3;
  const gpio_config_t config = {
      .pin_bit_mask = BIT(GPIO_NUM_3),
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
  };
  gpio_config(&config);
  esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_pin), ESP_GPIO_WAKEUP_GPIO_HIGH);
  
  Log.noticeln("Entering deep sleep mode!");

  esp_deep_sleep_start();
}

void loop() {}
