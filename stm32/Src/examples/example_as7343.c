/**
 * @example example_pcap02.c
 *
 * Prints the C1 / C0 (single floating) capacitance ratio from the PCAP02
 * capacitance to digital sensor. C0 is the reference capacitor connected across
 * PC0 and PC1 of the plugin module. C1 is the capacitance measured across the
 * PC2 and PC3 ports.
 *
 * @author Jack Lin
 * @date 2025-08-18
 */

// Includes
#include <stdio.h>

#include "as7343.h"
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

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
  AS7343Init();
  AS7343LEDOn();  // Turning LED on for better readings with inadequate light
                  // source

  while (1) {
    HAL_Delay(1000);

    AS7343Data sensorReadings = {};
//     AS7343DataTest sensorReadings = {};
    APP_LOG(TS_OFF, VLEVEL_M, "Getting Measurements\r\n");
    AS7343GetMeasurement(&sensorReadings);

    APP_LOG(TS_OFF, VLEVEL_M, "Purple: %hu\r\n",
            sensorReadings.channelCombined[CH_PURPLE_F1_405NM]);  // Purple
    APP_LOG(
        TS_OFF, VLEVEL_M, "Dark Blue: %hu\r\n",
        sensorReadings.channelCombined[CH_DARK_BLUE_F2_425NM]);  // Dark Blue
    APP_LOG(TS_OFF, VLEVEL_M, "Blue: %hu\r\n",
            sensorReadings.channelCombined[CH_BLUE_FZ_450NM]);  // Blue
    APP_LOG(
        TS_OFF, VLEVEL_M, "Light Blue: %hu\r\n",
        sensorReadings.channelCombined[CH_LIGHT_BLUE_F3_475NM]);  // Light Blue
    APP_LOG(TS_OFF, VLEVEL_M, "Cyan: %hu\r\n",
            sensorReadings.channelCombined[CH_BLUE_F4_515NM]);  // Cyan
    APP_LOG(TS_OFF, VLEVEL_M, "Green1: %hu\r\n",
            sensorReadings.channelCombined[CH_GREEN_F5_550NM]);  // Green1
    APP_LOG(TS_OFF, VLEVEL_M, "Green2: %hu\r\n",
            sensorReadings.channelCombined[CH_GREEN_FY_555NM]);  // Green2
    APP_LOG(TS_OFF, VLEVEL_M, "Orange: %hu\r\n",
            sensorReadings.channelCombined[CH_ORANGE_FXL_600NM]);  // Orange
    APP_LOG(TS_OFF, VLEVEL_M, "Brown: %hu\r\n",
            sensorReadings.channelCombined[CH_BROWN_F6_640NM]);  // Brown
    APP_LOG(TS_OFF, VLEVEL_M, "Red: %hu\r\n",
            sensorReadings.channelCombined[CH_RED_F7_690NM]);  // Red
    APP_LOG(TS_OFF, VLEVEL_M, "Dark Red: %hu\r\n",
            sensorReadings.channelCombined[CH_DARK_RED_F8_745NM]);  // Dark Red
    APP_LOG(TS_OFF, VLEVEL_M, "NIR: %hu\r\n",
            sensorReadings.channelCombined[CH_NIR_855NM]);  // NIR
    APP_LOG(TS_OFF, VLEVEL_M, "VIS: %hu\r\n",
            (sensorReadings.channelCombined[CH_VIS_1]) +  (sensorReadings.channelCombined[CH_VIS_2]) +  (sensorReadings.channelCombined[CH_VIS_3]) );  // NIR
    AS7343LEDOff();
  }
}
