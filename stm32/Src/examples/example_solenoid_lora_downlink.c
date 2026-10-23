#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "LmHandler.h"
#include "adc.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "lora_downlink.h"
#include "lora_size.h"
#include "payload.h"
#include "rtc.h"
#include "sensors.h"
#include "solenoid.h"
#include "sys_app.h"
#include "usart.h"

static uint8_t TestAppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

static LmHandlerAppData_t TestAppData = {0, 0, TestAppDataBuffer};

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  // MX_LPUART1_UART_Init();
  MX_I2C1_Init();
  SystemApp_Init();

  // init lorawan
  MX_LoRaWAN_Init();

  // Get max LoRaWAN payload size
  LoRaMacRegion_t region = 0;
  LmHandlerGetActiveRegion(&region);
  int8_t dr = 0;
  LmHandlerGetTxDatarate(&dr);
  uint8_t max_payload_size = lorawan_max_payload(region, dr);

  APP_LOG(TS_OFF, VLEVEL_H, "example_solenoid_lora_downlink.c.\r\n");

  SolenoidInit(16, SOLENOID_TYPE_LATCH_NO);
  SolenoidInit(18, SOLENOID_TYPE_LATCH_NO);
  SolenoidInit(19, SOLENOID_TYPE_HOLD_NO);
  SolenoidInit(21, SOLENOID_TYPE_HOLD_NC);

  SysTime_t current_time;
  SysTime_t previous_uplink = {.Seconds = 0, .SubSeconds = 0};

  while (1) {
    MX_LoRaWAN_Process();

    // Forcing an empty payload through to open receive windows (class A)
    current_time = SysTimeGet();
    if (current_time.Seconds - previous_uplink.Seconds >= 10) {
      PayloadStatus payload_status = PAYLOAD_OK;
      payload_status = FormatPayload(TestAppData.Buffer, max_payload_size,
                                     (size_t *)&TestAppData.BufferSize);
      if (payload_status != PAYLOAD_OK) {
        APP_LOG(TS_OFF, VLEVEL_H, "Failed to format payload (%d).\r\n",
                payload_status);
      }
      TestAppData.Port = 200;  // currently unused port
      LmHandlerErrorStatus_t lmstatus;
      lmstatus = LmHandlerSend(&TestAppData,
                               LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, false);
      if (lmstatus == LORAMAC_HANDLER_SUCCESS) {
        APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
      } else {
        APP_LOG(TS_OFF, VLEVEL_M, "Could not send request\r\n");
        APP_LOG(TS_OFF, VLEVEL_M, "LmHandlerSend status = %d\r\n", lmstatus);
      }

      previous_uplink = SysTimeGet();
    }
  }
  /* USER CODE END 3 */
}