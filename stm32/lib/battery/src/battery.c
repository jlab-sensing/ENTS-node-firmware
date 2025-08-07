/**
 * @file battery.c
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-16
 */

#include "battery.h"

#include "adc.h"
#include "board.h"

/** Voltage reference in mV */
#define VOLTAGE_REF 3300

/** Full scale range of ADC, calculated with (2^nbits)-1 */
#define FULL_SCALE 4095

/** Numerator of voltage scale */
#define VOLTAGE_SCALE_NUM 5100
/** Denominator of voltage scale */
#define VOLTAGE_SCALE_DEN 3112

/** Private variable that stores battery voltage */
volatile uint32_t adc_reading = 0;

HAL_StatusTypeDef battery_init(void) {
  HAL_StatusTypeDef rc = HAL_OK;

  // Calibrate and start conversion process
  rc = HAL_ADCEx_Calibration_Start(&hadc);
  if (rc != HAL_OK) Error_Handler();

  // Start continuous measurements
  rc = HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_reading, 1);
  if (rc != HAL_OK) Error_Handler();

  return rc;
}

unsigned int battery_voltage(void) {
  // Adjust to real voltage value
  unsigned int voltage = (unsigned int)adc_reading << 1;
  voltage *= VOLTAGE_REF;
  voltage /= FULL_SCALE;

  // Calibrate to voltage level
  voltage *= VOLTAGE_SCALE_NUM;
  voltage /= VOLTAGE_SCALE_DEN;

  return voltage;
}
