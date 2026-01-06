/**
 * @file lora_size.h
 *
 * @brief Library for calculating LoRaWAN payload sizes.
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2026-01-02
 */

#ifndef STM32_INC_LORA_SIZE_H_
#define STM32_INC_LORA_SIZE_H_

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the maximum LoRaWAN MACPayload size.
 *
 * Does not account for MAC commands or FOpts field. A recommended overhead is
 * 20 bytes (13 bytes for MAC commands + 7 bytes for FOpts).
 *
 * Use regions defineed here: https://github.com/STMicroelectronics/stm32-mw-lorawan/blob/bde8ea838f49d7d2b3657ae5757dc6f1801fd6bd/Mac/LoRaMacInterfaces.h#L956-L998
 *
 * @param region LoRaWAN region.
 * @param dr Data rate (0-15).
 * @return Maximum MACPayload size in bytes.
 */
uint8_t lorawan_max_payload(unsigned int region, int8_t dr);

#ifdef __cplusplus
}
#endif


#endif  // STM32_INC_LORA_SIZE_H_
