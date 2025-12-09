/**
 * @example example_gui.c
 *
 * Demonstrates the functionality of the GUI. Exposes the communication
 * interface for comunication with the desktop gui.
 *
 * @author Ahmed Hassan Falah
 * @date N/A
 */

// system includes
#include "gpio.h"
#include "rtc.h"
#include "sys_app.h"

// user includes
#include "board.h"
#include "userConfig.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  /*Initialize timer and RTC*/
  /*Have to be initilized in example files because LoRaWan cannot be initialized
   * like in main*/
  // __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  // UTIL_TIMER_Init();
  SystemApp_Init();
  /* USER CODE BEGIN 2 */
  // UserConfig_InterruptInit();  // Initialize UART for interrupt mode
  /* USER CODE END 2 */
  // uint8_t length_buf;
  // UserConfig_ProcessDataPolling();
  UserConfig_InitAdvanceTrace();
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    // if (HAL_UART_Receive(&huart1, length_buf, 1, HAL_MAX_DELAY) == HAL_OK) {
    //     UserConfig_SendCurrentUserConfig();
    // }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
