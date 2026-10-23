/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "pin.h"
#include "stm32wlxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define __HAL_RCC_GPIOx_CLK_ENABLE(GPIO_PORT) \
if (GPIO_PORT == GPIOA){                      \
    __HAL_RCC_GPIOA_CLK_ENABLE();             \
}else if (GPIO_PORT == GPIOB){                \
    __HAL_RCC_GPIOB_CLK_ENABLE();             \
}else if (GPIO_PORT == GPIOC){                \
    __HAL_RCC_GPIOC_CLK_ENABLE();             \
}else if (GPIO_PORT ==  GPIOH){               \
    __HAL_RCC_GPIOH_CLK_ENABLE();             \
}
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_PREDIV_A ((1 << (15 - RTC_N_PREDIV_S)) - 1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1 << RTC_N_PREDIV_S) - 1)
#define VBAT_DIV_2_Pin GPIO_PIN_15
#define VBAT_DIV_2_GPIO_Port GPIOA
#define SDI12_DIR_Pin GPIO_PIN_9
#define SDI12_DIR_GPIO_Port GPIOB
#define USER_LED_Pin GPIO_PIN_5
#define USER_LED_GPIO_Port GPIOB
#define RF_CTRL3_Pin GPIO_PIN_3
#define RF_CTRL3_GPIO_Port GPIOC
#define ESP32_EN_Pin GPIO_PIN_0
#define ESP32_EN_GPIO_Port GPIOA
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOB
#define RF_CTRL2_Pin GPIO_PIN_5
#define RF_CTRL2_GPIO_Port GPIOC
#define SDI12_TX_Pin GPIO_PIN_1
#define SDI12_TX_GPIO_Port GPIOC
#define SDI12_RX_Pin GPIO_PIN_0
#define SDI12_RX_GPIO_Port GPIOC
#define RF_CTRL1_Pin GPIO_PIN_4
#define RF_CTRL1_GPIO_Port GPIOC
#define USART2_RX_CP2102_Pin GPIO_PIN_3
#define USART2_RX_CP2102_GPIO_Port GPIOA
#define USART2_TX_CP2102_Pin GPIO_PIN_2
#define USART2_TX_CP2102_GPIO_Port GPIOA

// Interrupt for PCAP02 INTN pin, which signals result ready
#define PCAP02_INTN_Pin GPIO_PIN_10
#define PCAP02_INTN_GPIO_Port GPIOA
#define PCAP02_INTN_EXTI_IRQn EXTI15_10_IRQn

// YF-S201C water flow meter
#define YFS201C_Pin GPIO_PIN_15
#define YFS201C_GPIO_Port GPIOA
#define YFS201C_EXTI_IRQn EXTI15_10_IRQn

// D10 water meter
#define D10_Pin GPIO_PIN_15
#define D10_GPIO_Port GPIOA
#define D10_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

// Physical 40-pin header map
#define PHYS_PIN_3_PORT GPIOA
#define PHYS_PIN_3_PIN GPIO_PIN_10
#define PHYS_PIN_5_PORT GPIOA
#define PHYS_PIN_5_PIN GPIO_PIN_9
#define PHYS_PIN_8_PORT GPIOB
#define PHYS_PIN_8_PIN GPIO_PIN_6
#define PHYS_PIN_10_PORT GPIOB
#define PHYS_PIN_10_PIN GPIO_PIN_7
#define PHYS_PIN_15_PORT GPIOB
#define PHYS_PIN_15_PIN GPIO_PIN_9
#define PHYS_PIN_16_PORT GPIOB
#define PHYS_PIN_16_PIN GPIO_PIN_13
#define PHYS_PIN_18_PORT GPIOA
#define PHYS_PIN_18_PIN GPIO_PIN_15
#define PHYS_PIN_19_PORT GPIOB
#define PHYS_PIN_19_PIN GPIO_PIN_15
#define PHYS_PIN_21_PORT GPIOB
#define PHYS_PIN_21_PIN GPIO_PIN_14
#define PHYS_PIN_22_PORT GPIOB
#define PHYS_PIN_22_PIN GPIO_PIN_3
#define PHYS_PIN_23_PORT GPIOB
#define PHYS_PIN_23_PIN GPIO_PIN_10
#define PHYS_PIN_24_PORT GPIOB
#define PHYS_PIN_24_PIN GPIO_PIN_4

#define PHYS_PIN_27_PORT 1
#define PHYS_PIN_27_PIN 0
#define PHYS_PIN_29_PORT 1
#define PHYS_PIN_29_PIN 1
#define PHYS_PIN_31_PORT 1
#define PHYS_PIN_31_PIN 2
#define PHYS_PIN_33_PORT 1
#define PHYS_PIN_33_PIN 3
#define PHYS_PIN_35_PORT 1
#define PHYS_PIN_35_PIN 4
#define PHYS_PIN_37_PORT 1
#define PHYS_PIN_37_PIN 5
#define PHYS_PIN_28_PORT 1
#define PHYS_PIN_28_PIN 6
#define PHYS_PIN_36_PORT 1
#define PHYS_PIN_36_PIN 7






/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
