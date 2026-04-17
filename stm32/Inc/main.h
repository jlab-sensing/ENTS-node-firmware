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
#include "stm32wlxx_hal.h"
#include "board.h"

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

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
