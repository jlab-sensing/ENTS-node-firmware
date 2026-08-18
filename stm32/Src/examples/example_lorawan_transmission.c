/**
 * @example example_lorawan_transmission
 *
 * Prints the C1 / C0 (single floating) capacitance ratio from the PCAP02
 * capacitance to digital sensor. C0 is the reference capacitor connected across
 * PC0 and PC1 of the plugin module. C1 is the capacitance measured across the
 * PC2 and PC3 ports.
 *
 * @author Eric Tran
 * @date 2025-08-18
 */

// Includes
#include <stdio.h>

#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "lora_app.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"
/////////
#include "LmHandler.h"
#include "LmHandlerTypes.h"
#include "payload.h"

/**
 * @brief User application buffer
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
 * @brief User application data structure
 */
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};

///////////

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  // NOTE: MX_I2C1_Init() must be called prior to AS7343Init();.
  LoRaWAN_Init();
  //   AS7343LEDOn(); // Turning LED on for better readings with inadequate
  //   light source
  // AppData.BufferSize = 3;
  // AppData.Buffer[0] = 1;
  // AppData.Buffer[1] = 2;
  // AppData.Buffer[2] = 3;

  // AppData.Port = LORAWAN_SPS_MEAS_GENERIC_PORT;
  // LmHandlerErrorStatus_t lmstatus;

  while (1) {
    HAL_Delay(1000);

    // const size_t buffer_size = 256;
    // size_t buffer_len = 3;
    // uint8_t buffer[buffer_size] = {1,2,3};

    // // get payload (pegged at 512)
    // PayloadStatus payload_status = PAYLOAD_OK;
    // payload_status = FormatPayload(buffer, buffer_size, &buffer_len);
    // if (payload_status == PAYLOAD_ERROR) {
    //   APP_LOG(TS_OFF, VLEVEL_M, "Error formatting payload\r\n");
    //   return;
    // } else if (payload_status == PAYLOAD_NO_DATA) {
    //   APP_LOG(TS_OFF, VLEVEL_M, "No data to send\r\n");
    //   return;
    // }

    // if (LmHandlerIsBusy()) {
    // APP_LOG(TS_ON, VLEVEL_M, "LmHandler is busy\r\n");
    // HAL_Delay(1000);
    // continue;
    // }

    // lmstatus =
    //       LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
    //       false);
    //   if (lmstatus == LORAMAC_HANDLER_SUCCESS) {
    //     APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
    //   } else {
    //     APP_LOG(TS_OFF, VLEVEL_M, "Could not send request\r\n");
    //     APP_LOG(TS_OFF, VLEVEL_M, "LmHandlerSend status = %d\r\n", lmstatus);
    // }
  }
}
