/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc_if.c
  * @author  MCD Application Team
  * @brief   Read status related to the chip (battery level, VREF, chip temperature)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "adc_if.h"
#include "sys_app.h"
#include "board.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/**
  * @brief ADC handle
  */
extern ADC_HandleTypeDef hadc;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define TEMPSENSOR_TYP_CAL1_V          (( int32_t)  760)        /*!< Internal temperature sensor, parameter V30 (unit: mV). Refer to device datasheet for min/typ/max values. */
#define TEMPSENSOR_TYP_AVGSLOPE        (( int32_t) 2500)        /*!< Internal temperature sensor, parameter Avg_Slope (unit: uV/DegCelsius). Refer to device datasheet for min/typ/max values. */

/* USER CODE BEGIN PD */

/** Number of ADC channels configured */
#define ADC_CHANNELS 3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/** Volatile DMA storage of ADC readings */
uint32_t adc_values_dma[ADC_CHANNELS];

/** Static storage of ADC readings */
uint32_t adc_values[ADC_CHANNELS];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief This function reads the ADC channel
  * @param channel channel number to read
  * @return adc measured level value
  */
static uint32_t ADC_ReadChannels(uint32_t channel);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */

void SYS_InitMeasurement(void)
{
  /* USER CODE BEGIN SYS_InitMeasurement_1 */
  HAL_StatusTypeDef rc = HAL_OK;
  /* USER CODE END SYS_InitMeasurement_1 */
  hadc.Instance = ADC;
  /* USER CODE BEGIN SYS_InitMeasurement_2 */

  // Run ADC initialization code  
  MX_ADC_Init();

  /* Start Calibration */
  rc = HAL_ADCEx_Calibration_Start(&hadc);
  if (rc != HAL_OK) Error_Handler();

  // Start ADC conversions with DMA
  rc = HAL_ADC_Start_DMA(&hadc, adc_values_dma, ADC_CHANNELS);
  if (rc != HAL_OK) Error_Handler();

  /* USER CODE END SYS_InitMeasurement_2 */
}

void SYS_DeInitMeasurement(void)
{
  /* USER CODE BEGIN SYS_DeInitMeasurement_1 */
  HAL_StatusTypeDef rc = HAL_OK;
  HAL_ADC_Stop_DMA(&hadc);
  if (rc != HAL_OK) Error_Handler();

  // Deinit ADC
  HAL_ADC_DeInit(&hadc);
  /* USER CODE END SYS_DeInitMeasurement_1 */
}

int16_t SYS_GetTemperatureLevel(void)
{
  /* USER CODE BEGIN SYS_GetTemperatureLevel_1 */

  /* USER CODE END SYS_GetTemperatureLevel_1 */
  __IO int16_t temperatureDegreeC = 0;
  uint32_t measuredLevel = 0;
  uint16_t batteryLevelmV = SYS_GetBatteryLevel();

  measuredLevel = ADC_ReadChannels(1);

  /* convert ADC level to temperature */
  /* check whether device has temperature sensor calibrated in production */
  if (((int32_t)*TEMPSENSOR_CAL2_ADDR - (int32_t)*TEMPSENSOR_CAL1_ADDR) != 0)
  {
    /* Device with temperature sensor calibrated in production:
       use device optimized parameters */
    temperatureDegreeC = __LL_ADC_CALC_TEMPERATURE(batteryLevelmV,
                                                   measuredLevel,
                                                   LL_ADC_RESOLUTION_12B);
  }
  else
  {
    /* Device with temperature sensor not calibrated in production:
       use generic parameters */
    temperatureDegreeC = __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(TEMPSENSOR_TYP_AVGSLOPE,
                                                              TEMPSENSOR_TYP_CAL1_V,
                                                              TEMPSENSOR_CAL1_TEMP,
                                                              batteryLevelmV,
                                                              measuredLevel,
                                                              LL_ADC_RESOLUTION_12B);
  }

  /* from int16 to q8.7*/
  temperatureDegreeC <<= 8;

  return (int16_t) temperatureDegreeC;
  /* USER CODE BEGIN SYS_GetTemperatureLevel_2 */

  /* USER CODE END SYS_GetTemperatureLevel_2 */
}

uint16_t SYS_GetBatteryLevel(void)
{
  /* USER CODE BEGIN SYS_GetBatteryLevel_1 */

  /* USER CODE END SYS_GetBatteryLevel_1 */
  uint16_t batteryLevelmV = 0;
  uint32_t measuredLevel = 0;

  measuredLevel = ADC_ReadChannels(2);

  if (measuredLevel == 0)
  {
    batteryLevelmV = 0;
  }
  else
  {
    if ((uint32_t)*VREFINT_CAL_ADDR != (uint32_t)0xFFFFU)
    {
      /* Device with Reference voltage calibrated in production:
         use device optimized parameters */
      batteryLevelmV = __LL_ADC_CALC_VREFANALOG_VOLTAGE(measuredLevel,
                                                        ADC_RESOLUTION_12B);
    }
    else
    {
      /* Device with Reference voltage not calibrated in production:
         use generic parameters */
      batteryLevelmV = (VREFINT_CAL_VREF * 1510) / measuredLevel;
    }
  }

  return batteryLevelmV;
  /* USER CODE BEGIN SYS_GetBatteryLevel_2 */

  /* USER CODE END SYS_GetBatteryLevel_2 */
}

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */

/**
 * @brief ADC callback on the completion of a conversion
 * 
 * Copies measurements from buffer into non-volatile storage for later access.
 * 
 * @param hadc Handle to ADC
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  memcpy(adc_values, adc_values_dma, ADC_CHANNELS);
}

/* USER CODE END PrFD */

static uint32_t ADC_ReadChannels(uint32_t channel)
{
  /* USER CODE BEGIN ADC_ReadChannels_1 */

  /* USER CODE END ADC_ReadChannels_1 */
  return adc_values[channel];
  /* USER CODE BEGIN ADC_ReadChannels_2 */

  /* USER CODE END ADC_ReadChannels_2 */
}
