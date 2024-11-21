#include <Arduino.h>
#include <esp_sleep.h>


#define STM32_SIGNAL_PIN 3  // GPIO I03

void setup() {
    Serial.begin(115200);

    esp_deep_sleep_enable_gpio_wakeup(1ULL << STM32_SIGNAL_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
    
    Serial.println("Wake-up caused by external signal on pin I03.");
    delay(1000);

    Serial.println("Entering deep sleep...");
    delay(100);

    esp_deep_sleep_start();
}

void loop() {
}
