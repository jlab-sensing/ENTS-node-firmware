/**
 * @example example_lora_downlink.c
 *
 * Prints downlink information if one was scheduled before a periodic uplink
 * was sent.
 *
 * @author Eric Tran
 * @date 2026-8-3
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "LmHandler.h"
#include "app_lorawan.h"
#include "lora_downlink.h"
#include "lora_size.h"
#include "payload.h"
#include "sensors.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

/** Global variable for all return codes */
HAL_StatusTypeDef rc;

static uint8_t TestAppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

static LmHandlerAppData_t TestAppData = {0, 0, TestAppDataBuffer};

/**
 * @brief Entry point for battery test
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  // init system app
  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  // init lorawan
  MX_LoRaWAN_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,
          "Example Lora Downlink (%s), compile on %s %s\r\n", __FILE__,
          __DATE__, __TIME__);

  // Infinite loop

  // Get max LoRaWAN payload size
  LoRaMacRegion_t region = 0;
  int8_t dr = 0;
  LmHandlerGetTxDatarate(&dr);
  uint8_t max_payload_size = lorawan_max_payload(region, dr);

  while (1) {
    MX_LoRaWAN_Process();
    HAL_Delay(DELAY);
    // Forcing an empty payload through to receive downlink
    FormatPayload(TestAppData.Buffer, max_payload_size,
                  (size_t *)&TestAppData.BufferSize);
    LmHandlerSend(&TestAppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, false);

    // prints the stored downlink data when new data is available
    if (downlinkIsNewDataReady()) {
      // storing app data
      const LmHandlerAppData_t latestAppData = getDownlinkData();
      APP_LOG(TS_OFF, VLEVEL_H, "Stored AppData:\r\n");
      APP_LOG(TS_OFF, VLEVEL_H, "OnRxData Port: %u\r\n", latestAppData.Port);
      APP_LOG(TS_OFF, VLEVEL_H, "OnRxData BufferSize: %u\r\n",
              latestAppData.BufferSize);
      APP_LOG(TS_OFF, VLEVEL_H, "OnRxData Buffer (hex): ");
      for (int i = 0; i < latestAppData.BufferSize; i++) {
        APP_LOG(TS_OFF, VLEVEL_H, "%02X", latestAppData.Buffer[i]);
      }
      APP_LOG(TS_OFF, VLEVEL_H, "\r\n");
    }
  }
}
