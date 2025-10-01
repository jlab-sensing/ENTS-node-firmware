/**
 * Example code to demonstrate the controller library.
 *
 * Upon startup with esp32 will attempt to connect to the WiFi network as
 * set by the userconfig. The esp32 should have the release code with WiFi
 * enabled.
 *
 * @author Ahmed
 * @date 2025-08-05
 */

#include "adc.h"
#include "board.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "stm32_adv_trace.h"
#include "stm32_systime.h"
#include "usart.h"
#include "userConfig.h"

/** Timeout for i2c communication with esp32 */
// unsigned int g_controller_i2c_timeout = 10000;

// uint8_t ControllerWiFiInit(const char *ssid, const char *passwd) {
//     // get reference to tx and rx buffers
//     Buffer *tx = ControllerTx();
//     Buffer *rx = ControllerRx();

//     tx->data = "A";
//     tx->len = sizeof("A");

//     // send transaction
//     ControllerStatus status = CONTROLLER_SUCCESS;
//     status = ControllerTransaction(g_controller_i2c_timeout);
//     if (status != CONTROLLER_SUCCESS) {
//       return -1;
//     }

//     // check for errors
//     if (rx->len == 0) {
//       return -1;
//     }

// if(rx->data == "A"){

// }
//   }

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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  SystemApp_Init();

  ControllerInit();

  /* USER CODE BEGIN 2 */
  const char* ssid = "ABC";
  const char* passwd = "123";

  APP_LOG(TS_OFF, VLEVEL_M, "Connecting to %s. Status: ", ssid);
  uint8_t wifi_status = ControllerWiFiConnect(ssid, passwd);
  APP_LOG(TS_OFF, VLEVEL_M, "%d\r\n", wifi_status);

  while (1) {
  }
}
