/**
 * @example example_microsd.c
 *
 * Example to save an adc reading to the microsd card.
 *
 * Requires the ESP32 to be flashed with the latest `release` code.
 *
 * This example does not use the user config.
 */

// stdlib includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// system includes
#include "adc.h"
#include "board.h"
#include "controller/controller.h"
#include "controller/microsd.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "sensor.h"
#include "sensors.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "ads.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  TCA9535Init(false);
  SystemApp_Init();

  APP_PRINTF("example_microsd, compiled on %s %s\r\n", __DATE__, __TIME__);

  ControllerInit();  // boot ESP32 in order to save to the micro SD card

  // Send wakeup to ESP32
  HAL_Delay(1000);
  ControllerWakeup();
  HAL_Delay(1000);

  if (!ControllerPowerWakeup()) {  // while
    APP_LOG(TS_OFF, VLEVEL_M, "Error waking up ESP32!\r\n");
    // HAL_Delay(5000);
  }
  APP_LOG(TS_OFF, VLEVEL_M, "Successfully woke up ESP32.\r\n");

  if (ADC_init()) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to initailize ADS1219.\r\n");
    while (1);
  }

  uint8_t encoded_measurement[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
  size_t encoded_measurement_length;
  SysTime_t timestamp;
  SensorMeasurement decoded_measurement = SensorMeasurement_init_zero;

  while (1) {
    timestamp = SysTimeGet();
    APP_PRINTF("Time: %u.%hd\r\n", timestamp.Seconds, timestamp.SubSeconds);

    APP_PRINTF("Taking voltage measurement... ");
    encoded_measurement_length =
        ADC_measureVoltage(encoded_measurement, timestamp, 0);
    if (encoded_measurement_length == -1) {
      APP_PRINTF("Error reading voltage.\r\n");
      continue;
    }
    APP_PRINTF("Success.\r\n");

    APP_PRINTF("\tEncoded data (%d bytes): 0x", encoded_measurement_length);
    for (int i = 0; i < encoded_measurement_length; i++) {
      APP_PRINTF("%02X", encoded_measurement[i]);
    }
    APP_PRINTF("\r\n");

    DecodeSensorMeasurement(encoded_measurement, encoded_measurement_length,
                            &decoded_measurement);
    APP_PRINTF("\tvalue.decimal: %lf\r\n", decoded_measurement.value.decimal);
    APP_PRINTF("\tmeta.cell_id: %u\r\n", decoded_measurement.meta.cell_id);
    APP_PRINTF("\tmeta.logger_id: %u\r\n", decoded_measurement.meta.logger_id);
    APP_PRINTF("\tmeta.ts: %u\r\n", decoded_measurement.meta.ts);
    APP_PRINTF("\tidx: %u\r\n", decoded_measurement.idx);

    APP_PRINTF(
        "Sending encoded measurement to ESP32 for saving to micro SD... ");

    ControllerMicroSDSave(encoded_measurement, encoded_measurement_length,
                          NULL);

    // encoded_measurement_length =
    //     ADC_measureVoltage(encoded_measurement, timestamp, 0);
    // if (encoded_measurement_length == -1) {
    //   APP_PRINTF("Error reading voltage.\r\n");
    //   continue;
    // }
    // APP_PRINTF("Success.\r\n");

    HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}
