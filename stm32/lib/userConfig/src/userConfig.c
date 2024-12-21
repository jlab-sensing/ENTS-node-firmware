/**
 ******************************************************************************
 * @file     userConfig.h
 * @author   Ahmed Hassan Falah
 * @brief    This file contains all the function definitions for
 *           the userConfig library.
 *
 * @date     10/13/2024
 ******************************************************************************
 * Copyright [2024] <Ahmed Hassan Falah>
 */

/* Includes ------------------------------------------------------------------*/
#include "userConfig.h"
// variable to store each byte received from interrupt
static uint8_t charRx;
// Receive buffer to store encoded data
static uint8_t RX_Buffer[RX_BUFFER_SIZE];
// Starting address to write in FRAM
static uint16_t fram_addr = FRAM_START_ADDRESS;
// Acknowledgment message
uint8_t ack[] = "ACK";
// Static variable to store the loaded user configuration in RAM
static UserConfiguration loadedConfig;

// Initialize UART for interrupt-based receiving
void UserConfig_InterruptInit(void) {
  // Enable UART receive interrupt( read 1-byte )
  HAL_UART_Receive_IT(&huart1, &charRx, 1);
}

// Interrupt handler for UART receive
void UserConfig_ReceiveInterruptHandler(void) {
  // buffer to store length in bytes
  static uint8_t length_buf[2];
  // length of received data after converting it
  static uint16_t data_length = 0;
  static uint16_t index = 0;
  static bool length_received = false;

  if (!length_received) {
    length_buf[index++] = charRx;
    if (index == 2) {
      data_length = (length_buf[0] << 8) | length_buf[1];
      index = 0;
      length_received = true;

      if (data_length > RX_BUFFER_SIZE) {
        uint8_t error_msg[] = "Data too large";
        HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                          HAL_MAX_DELAY);
        length_received = false;
        HAL_UART_Receive_IT(&huart1, &charRx, 1);
        return;
      }
    }
  } else {
    // Accumulate incoming data into rx_buf based on received length
    RX_Buffer[index++] = charRx;
    if (index == data_length) {
      length_received = false;
      index = 0;
      HAL_UART_Transmit(&huart1, ack, sizeof(ack) - 1, HAL_MAX_DELAY);

      UserConfigStatus status =
          UserConfig_WriteToFRAM(fram_addr, RX_Buffer, data_length);
      if (status == USERCONFIG_OK) {
        uint8_t length_to_send[2] = {(data_length >> 8) & 0xFF,
                                     data_length & 0xFF};
        // Save encoded userconfig Length in the FRAM
        if (UserConfig_WriteToFRAM(USER_CONFIG_LEN_ADDR, length_to_send, 2) !=
            USERCONFIG_OK) {
          uint8_t error_msg[] = "FRAM Error";
          HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                            HAL_MAX_DELAY);
        }
        // Send encoded userconfig Length back to GUI
        HAL_UART_Transmit(&huart1, length_to_send, 2, HAL_MAX_DELAY);
        // Read the received encoded data from FRAM
        UserConfig_ReadFromFRAM(fram_addr, data_length, RX_Buffer);
        // Send the received data back to the GUI for confirmation
        HAL_UART_Transmit(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY);
      } else {
        uint8_t error_msg[] = "FRAM Error";
        HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                          HAL_MAX_DELAY);
      }
    }
  }

  HAL_UART_Receive_IT(&huart1, &charRx, 1);
}

// Polling method for processing received data
void UserConfig_ProcessDataPolling(void) {
  uint8_t length_buf[2];     // Buffer to store received data length in bytes
  uint16_t data_length = 0;  // Length received data

  // Poll to receive the length of the encoded data (2 bytes)
  if (HAL_UART_Receive(&huart1, length_buf, 2, HAL_MAX_DELAY) == HAL_OK) {
    // Convert 2 bytes to length
    data_length = (length_buf[0] << 8) | length_buf[1];
    // If data length exceeds buffer size, handle error
    if (data_length > RX_BUFFER_SIZE) {
      uint8_t error_msg[] = "Data too large";
      HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                        HAL_MAX_DELAY);
      return;
    }
    // Receive the actual encoded data of the received length
    if (HAL_UART_Receive(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY) ==
        HAL_OK) {
      // Send acknowledgment ("ACK") to the GUI
      HAL_UART_Transmit(&huart1, ack, sizeof(ack) - 1, HAL_MAX_DELAY);
      // Write the received data to FRAM
      UserConfigStatus status =
          UserConfig_WriteToFRAM(fram_addr, RX_Buffer, data_length);

      if (status == USERCONFIG_OK) {
        // Send the received data back to the GUI for confirmation
        uint8_t length_to_send[2] = {(data_length >> 8) & 0xFF,
                                     data_length & 0xFF};
        HAL_UART_Transmit(&huart1, length_to_send, 2, HAL_MAX_DELAY);
        UserConfig_ReadFromFRAM(fram_addr, data_length, RX_Buffer);
        HAL_UART_Transmit(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY);
      } else {
        // Handle FRAM write error
        uint8_t error_msg[] = "FRAM Error";
        HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                          HAL_MAX_DELAY);
      }
    }
  }
}

// Write data to FRAM
UserConfigStatus UserConfig_WriteToFRAM(uint16_t fram_addr, uint8_t *data,
                                        uint16_t length) {
  FramStatus status = FramWrite(fram_addr, data, length);
  return (status == FRAM_OK) ? USERCONFIG_OK : USERCONFIG_FRAM_ERROR;
}

// Read data from FRAM
UserConfigStatus UserConfig_ReadFromFRAM(uint16_t fram_addr, uint16_t length,
                                         uint8_t *data) {
  FramStatus status = FramRead(fram_addr, length, data);
  return (status == FRAM_OK) ? USERCONFIG_OK : USERCONFIG_FRAM_ERROR;
}

// Load user configuration data from FRAM to RAM.
UserConfigStatus UserConfigLoad(void) {
  uint16_t data_length = 0;
  uint8_t length_buf[2];

  // Read the length of the user configuration data from FRAM
  if (UserConfig_ReadFromFRAM(USER_CONFIG_LEN_ADDR, 2, length_buf) !=
      USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Convert length bytes to integer
  data_length = (length_buf[0] << 8) | length_buf[1];

  // Read the encoded configuration data from FRAM into RX_Buffer
  if (UserConfig_ReadFromFRAM(FRAM_START_ADDRESS, data_length, RX_Buffer) !=
      USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Decode the user configuration from RX_Buffer into loadedConfig struct
  if (DecodeUserConfiguration(RX_Buffer, data_length, &loadedConfig) !=
      USERCONFIG_OK) {
    // Return an error if decoding fails
    return USERCONFIG_DECODE_ERROR;
  }

  return USERCONFIG_OK;  // Return success if decoding is successful
}

// Get a reference to the loaded user configuration data in RAM.
const UserConfiguration *UserConfigGet(void) { return &loadedConfig; }
