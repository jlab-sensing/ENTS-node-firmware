#ifndef STM32_LIB_CONTROLLER_INC_CONTROLLER_H_
#define STM32_LIB_CONTROLLER_INC_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Shared initialization for all esp32  modules
 * 
 * Allocates memory to the tx and rx buffers.
 * 
 * @todo Add check for communication with the esp32
 */
void ControllerInit(void);

/**
 * @brief Shared deinitialize for all esp32 modules
 * 
 * Free memory associated with the tx and rx buffers.
 */
void ControllerDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // STM32_LIB_CONTROLLER_INC_CONTROLLER_H_