/**
 * @example example_adv_trace.c
 *
 * Example showing transmission and reception of data using the Advance Trace
 * library.
 */

// system includes
#include "board.h"
#include "gpio.h"
#include "rtc.h"
#include "stm32_adv_trace.h"
#include "stm32_systime.h"
#include "usart.h"

// NOLINTBEGIN
void RxCallback(uint8_t *pData, uint16_t Size, uint8_t Error) {
  if (Error == 0 && Size == 1) {
    char receivedChar = *pData;
    uint8_t sendACK = 'A';
    uint8_t sendNACK = 'N';

    // Process the received character
    if (receivedChar == 1) {
      // Send ACK to the sender
      HAL_UART_Transmit(&huart1, &sendACK, 1, HAL_MAX_DELAY);
    } else {
      // Send NACK to the sender
      HAL_UART_Transmit(&huart1, &sendNACK, 1, HAL_MAX_DELAY);
    }

    // Start receiving the next character
    UTIL_ADV_TRACE_StartRxProcess(RxCallback);
  } else {
    // re-initialize the receive process
    UTIL_ADV_TRACE_StartRxProcess(RxCallback);
  }
}
// NOLINTEND

void InitAdvanceTrace() {
  // Configure Advance Trace module
  UTIL_ADV_TRACE_Init();

  // callback function
  UTIL_ADV_TRACE_StartRxProcess(RxCallback);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  UTIL_TIMER_Init();
  /* USER CODE BEGIN 2 */
  // UserConfig_InterruptInit();  // Initialize UART for interrupt mode

  // sprintf(uart_buf, "API Port: %lu\r\n", config->API_Endpoint_Port);
  /*HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf,
   strlen(uart_buf), HAL_MAX_DELAY); */
  /* USER CODE END 2 */
  InitAdvanceTrace();
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
