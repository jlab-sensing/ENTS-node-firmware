/**
 * @file test_ads.c
 * @brief Tests the ads library
 */

#include <stdio.h>
#include <unity.h>

#include "ads.h"
#include "board.h"
#include "fifo.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

static UserConfiguration cfg = UserConfiguration_init_zero;

// Calibration values. Leave as 1 0 1 0 to read raw values.
static double v_m = 1, v_b = 0, i_m = 1, i_b = 0;

void setUp(void) {
  // A1   A0   I2C ADDRESS
  // DGND DGND 100 0000 (0x40)
  // DGND DVDD 100 0001
  // DGND SDA  100 0010
  // DGND SCL  100 0011
  // DVDD DGND 100 0100
  // DVDD DVDD 100 0101
  // DVDD SDA  100 0110
  // DVDD SCL  100 0111
  // SDA  DGND 100 1000
  // SDA  DVDD 100 1001
  // SDA  SDA  100 1010
  // SDA  SCL  100 1011
  // SCL  DGND 100 1100
  // SCL  DVDD 100 1101
  // SCL  SDA  100 1110
  // SCL  SCL  100 1111
  cfg.enabled_sensors_multiple_count = 2;
  cfg.enabled_sensors_multiple[0].cell_id = 200;
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_Voltage;
  cfg.enabled_sensors_multiple[0].index = 0x40 << 1;
  cfg.enabled_sensors_multiple[1].cell_id = 200;
  cfg.enabled_sensors_multiple[1].enabled_sensor = EnabledSensor_Current;
  cfg.enabled_sensors_multiple[1].index = 0x40 << 1;
}

void tearDown(void) {}

void test_ADC_Init(void) {
  HAL_StatusTypeDef ret;
  ret = ADC_init(&(cfg.enabled_sensors_multiple[0]), v_m, v_b, i_m, i_b);

  TEST_ASSERT_EQUAL(HAL_OK, ret);
}

void test_ADC_readVoltage(void) {
  // Data size is larger than the buffer size
  double volt = ADC_readVoltage(&(cfg.enabled_sensors_multiple[0]));

  TEST_ASSERT_NOT_EQUAL(volt, -1);
}

void test_ADC_readCurrent(void) {
  // Data size is larger than the buffer size
  double curr = ADC_readCurrent(&(cfg.enabled_sensors_multiple[1]));

  TEST_ASSERT_NOT_EQUAL(curr, -1);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  UTIL_TIMER_Init();

  // wait for UART
  WaitForSerial();

  UNITY_BEGIN();
  // other tests depend on the following two
  RUN_TEST(test_ADC_Init);
  RUN_TEST(test_ADC_readVoltage);
  RUN_TEST(test_ADC_readCurrent);
  UNITY_END();
  /* USER CODE END 3 */
}
