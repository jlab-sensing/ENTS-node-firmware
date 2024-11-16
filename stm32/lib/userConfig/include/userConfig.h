/**
 ******************************************************************************
 * @file    userConfig.c
 * @author  Ahmed Hassan Falah
 * @brief   Soil Power Sensor userConfig library
 *          This file provides functions for managing user configuration data 
 *          via UART communication and storage in FRAM. It includes functions 
 *          for handling UART interrupts for receiving data, writing and 
 *          reading data from FRAM, and sending acknowledgment messages.
 * 
 * @details
 *          - The library initializes UART for interrupt-based receiving and 
 *            handles incoming data in an interrupt service routine.
 *          - It supports both interrupt-driven and polling methods for receiving 
 *            data.
 *          - The received data length is stored alongside the encoded data in 
 *            FRAM to facilitate data retrieval upon startup.
 *          - Requires app to be initialized.
 * 
 * @date    10/13/2024
 ******************************************************************************
 * copyright [2024] <Ahmed Hassan Falah>
 */

#ifndef LIB_USERCONFIG_INCLUDE_USERCONFIG_H_
#define LIB_USERCONFIG_INCLUDE_USERCONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "usart.h"
#include "fram.h"
#include "transcoder.h"
#include "string.h"
#include "stm32_systime.h"
#include "usart_if.h"
#include "soil_power_sensor.pb.h"


#define RX_BUFFER_SIZE UserConfiguration_size      // Maximum size of the received data buffer
// #define RX_BUFFER_SIZE 431                            // Maximum size of the received data buffer
#define FRAM_START_ADDRESS 1794                    // Starting address to write in FRAM
#define USER_CONFIG_LEN_ADDR 1792                  // Starting address to write in FRAM

typedef enum {
    USERCONFIG_OK,
    USERCONFIG_FRAM_ERROR,
    USERCONFIG_DECODE_ERROR
} UserConfigStatus;

/**
 ******************************************************************************
 * @brief    Initializes user configuration settings.
 * 
 *           This function sets up the UART receive interrupt.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
void UserConfig_InterruptInit(void);

/**
 ******************************************************************************
 * @brief    Processes incoming user configuration data via UART in polling mode.
 *
 *           This function checks for incoming user configuration data over UART 
 *           in polling mode and processes it as needed.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
void UserConfig_ProcessDataPolling(void);

/**
 ******************************************************************************
 * @brief    Handles received user configuration data via UART interrupt.
 *
 *           This function is called when user configuration data is received
 *           via UART interrupt. It processes and stores the data into FRAM for
 *           further use.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
void UserConfig_ReceiveInterruptHandler(void);

/**
 ******************************************************************************
 * @brief    Writes user configuration data to FRAM.
 *
 *           This function writes a specified amount of user configuration data 
 *           to the FRAM memory starting at a specified address.
 *
 * @param    fram_addr  Starting address in FRAM to write data.
 * @param    data       Pointer to the data to be written.
 * @param    length     Length of the data to be written.
 * @return   UserConfigStatus - USERCONFIG_OK if successful, error code otherwise.
 ******************************************************************************
 */
UserConfigStatus UserConfig_WriteToFRAM(uint16_t fram_addr,
                                        uint8_t *data, uint16_t length);

/**
 ******************************************************************************
 * @brief    Reads user configuration data from FRAM.
 *
 *           This function reads a specified amount of user configuration data 
 *           from the FRAM memory starting at a specified address.
 *
 * @param    fram_addr  Starting address in FRAM to read data from.
 * @param    length     Length of the data to be read.
 * @param    data       Pointer to the buffer to store the read data.
 * @return   UserConfigStatus - USERCONFIG_OK if successful, error code otherwise.
 ******************************************************************************
 */
UserConfigStatus UserConfig_ReadFromFRAM(uint16_t fram_addr,
                                         uint16_t length, uint8_t *data);

/**
 ******************************************************************************
 * @brief    Loads user configuration data from FRAM to RAM.
 *
 *           This function reads the stored user configuration data from FRAM,
 *           decodes it, and loads it into RAM. The data will be stored in a 
 *           static UserConfig structure.
 *
 * @param    void
 * @return   UserConfigStatus - USERCONFIG_OK if successful, error code otherwise.
 ******************************************************************************
 */
UserConfigStatus UserConfigLoad(void);

/**
 ******************************************************************************
 * @brief    Gets a reference to the loaded user configuration data in RAM.
 *
 *           This function returns a pointer to the UserConfig structure in RAM,
 *           allowing access to the loaded configuration without reading from FRAM.
 *
 * @param    void
 * @return   const UserConfig* - Pointer to the loaded UserConfig structure.
 ******************************************************************************
 */
const UserConfiguration* UserConfigGet(void);                      

#ifdef __cplusplus
}
#endif

#endif  // LIB_USERCONFIG_INCLUDE_USERCONFIG_H_
