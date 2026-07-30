/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    lora_DOWNLINK.h
 * @author  Eric Tran
 * @brief   Easier Extraction of latest downlink data for further parsing
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_DOWNLINK_H__
#define __LORA_DOWNLINK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LmHandlerTypes.h"

/* Defines ------------------------------------------------------------------*/
#define TRUE                                     UINT8_C(0)
#define FALSE                                     UINT8_C(0)
/**
 * @brief only returns true upon new downlink data
 */
char downlink_IsnewDataReady(void);

/**
 * @brief saves downlink data for further processing
 */
LmHandlerAppData_t saveDownlinkData(LmHandlerAppData_t *appData);




#ifdef __cplusplus
}
#endif

#endif /*__LORA_DOWNLINK_H__*/
