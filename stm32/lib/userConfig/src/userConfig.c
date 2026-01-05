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

#include "sys_app.h"

// Stores each byte received via UART interrupt.
static uint8_t charRx;
// Receive buffer for encoded configuration data.
static uint8_t RX_Buffer[RX_BUFFER_SIZE];
// Acknowledgment message sent to the GUI.
uint8_t ack[] = "ACK";
// Static variable to store the loaded user configuration in RAM
static UserConfiguration loadedConfig;
// Request flag: 0x01 = Receive, 0x02 = Send, 0x05 = Initial/Invalid.
static uint8_t RQFlag = 0x05;
// Flag indicating if the request flag byte has been received.
static bool checked;

#ifdef TEST_USER_CONFIG
const static UserConfiguration testConfig = {
    .logger_id = 200,
    .cell_id = 200,
    .Upload_method = Uploadmethod_WiFi,
    .Upload_interval = 10,
    .enabled_sensors_count = 2,
    .enabled_sensors = {EnabledSensor_Voltage, EnabledSensor_Current},
    // calibration values are taken from 2.2.3-033
    .Voltage_Slope = -0.00039326,
    .Voltage_Offset = 4.92916378e-05,
    .Current_Slope = -1.18693164e-10,
    .Current_Offset = 4.14518594e-05,
    .WiFi_SSID = "HARE_Lab",
    .WiFi_Password = "",
    .API_Endpoint_URL = "http://dirtviz.jlab.ucsc.edu/api/sensor/",
    // port is not used
    .API_Endpoint_Port = 80};
#endif  // TEST_USER_CONFIG

// Initialize Advance Trace for interrupt-based receiving
void UserConfig_InitAdvanceTrace() {
  // Set up the callback function
  UTIL_ADV_TRACE_StartRxProcess(UserConfig_RxCallback);
}

// UART interrupt callback function. Handles incoming data based on RQFlag.
void UserConfig_RxCallback(uint8_t *pData, uint16_t Size, uint8_t Error) {
  if (Error == 0 && Size == 1) {
    charRx = *pData;

    if (!checked) {
      RQFlag = charRx;  // First byte received
      checked = true;
      if (RQFlag == 0x01) return;  // Early return to avoid unnecessary checks.
    }

    switch (RQFlag) {
      case 0x01:  // Receive configuration data from GUI.
        UserConfig_InterruptHandler();
        break;
      case 0x02:  // Send current configuration data to GUI.
        UserConfig_SendCurrentUserConfig();
        break;
    }
  }
}

// Sends the current user configuration stored in FRAM to the GUI.
UserConfigStatus UserConfig_SendCurrentUserConfig(void) {
  uint16_t data_length;
  uint8_t length_buf[2];

  // Send ACK to the GUI
  HAL_UART_Transmit(&huart1, ack, sizeof(ack) - 1, HAL_MAX_DELAY);

  // Read the length of the encoded data from FRAM
  if (UserConfig_ReadFromFRAM(USER_CONFIG_LEN_ADDR, 2, length_buf) !=
      USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Convert length bytes to integer
  data_length = (length_buf[0] << 8) | length_buf[1];

  // Send the encoded data length to the GUI
  HAL_UART_Transmit(&huart1, length_buf, 2, HAL_MAX_DELAY);

  // Read the encoded data from FRAM
  if (UserConfig_ReadFromFRAM(USER_CONFIG_START_ADDRESS, data_length,
                              RX_Buffer) != USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Send the encoded data to the GUI
  HAL_UART_Transmit(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY);
  checked = false;
  RQFlag = 0x05;
  NVIC_SystemReset();
  return USERCONFIG_OK;
}

// Handles the reception of new configuration data from the GUI via interrupt.
void UserConfig_InterruptHandler(void) {
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
        checked = false;
      }
    }
  } else {
    // Accumulate incoming data into rx_buf based on received length
    RX_Buffer[index++] = charRx;
    if (index == data_length) {
      length_received = false;
      index = 0;
      HAL_UART_Transmit(&huart1, ack, sizeof(ack) - 1, HAL_MAX_DELAY);

      UserConfigStatus status = UserConfig_WriteToFRAM(
          USER_CONFIG_START_ADDRESS, RX_Buffer, data_length);

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
        UserConfig_ReadFromFRAM(USER_CONFIG_START_ADDRESS, data_length,
                                RX_Buffer);
        // Send the received data back to the GUI for confirmation
        HAL_UART_Transmit(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY);
        checked = false;
        NVIC_SystemReset();
      } else {
        uint8_t error_msg[] = "FRAM Error";
        HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                          HAL_MAX_DELAY);
      }
    }
  }
}

// Processes incoming user configuration data via UART using polling.
void UserConfig_ProcessDataPolling(void) {
  uint8_t length_buf[2];     // Buffer to store received data length in bytes
  uint16_t data_length = 0;  // Length received data

  // Poll to receive the length of the encoded data (2 bytes)
  if (HAL_UART_Receive(&huart1, length_buf, 2, 30000) == HAL_OK) {
    // Convert 2 bytes to length
    data_length = (length_buf[0] << 8) | length_buf[1];
    // Receive the actual encoded data of the received length
    if (HAL_UART_Receive(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY) ==
        HAL_OK) {
      // Send acknowledgment ("ACK") to the GUI
      HAL_UART_Transmit(&huart1, ack, sizeof(ack) - 1, HAL_MAX_DELAY);
      // Write the received data to FRAM
      UserConfigStatus status = UserConfig_WriteToFRAM(
          USER_CONFIG_START_ADDRESS, RX_Buffer, data_length);

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
        // Send the received data back to the GUI for confirmation
        HAL_UART_Transmit(&huart1, length_to_send, 2, HAL_MAX_DELAY);
        UserConfig_ReadFromFRAM(USER_CONFIG_START_ADDRESS, data_length,
                                RX_Buffer);
        HAL_UART_Transmit(&huart1, RX_Buffer, data_length, HAL_MAX_DELAY);
      } else {
        // Handle FRAM write error
        uint8_t error_msg[] = "FRAM Error";
        HAL_UART_Transmit(&huart1, error_msg, strlen((char *)error_msg),
                          HAL_MAX_DELAY);
      }
      checked = false;
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

// Load user configuration data from FRAM to RAM
UserConfigStatus UserConfigLoad(void) {
#ifdef TEST_USER_CONFIG
  return USERCONFIG_OK;
#else
  uint16_t data_length = 0;
  uint8_t length_buf[2];

  // Read the length of the user configuration data from FRAM
  if (UserConfig_ReadFromFRAM(USER_CONFIG_LEN_ADDR, 2, length_buf) !=
      USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Convert length bytes to integer
  data_length = (length_buf[0] << 8) | length_buf[1];

  if (data_length == 0) {
    return USERCONFIG_EMPTY_CONFIG;
  }

  // check the length for errors
  if (data_length > UserConfiguration_size) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Read the encoded configuration data from FRAM into RX_Buffer
  if (UserConfig_ReadFromFRAM(USER_CONFIG_START_ADDRESS, data_length,
                              RX_Buffer) != USERCONFIG_OK) {
    return USERCONFIG_FRAM_ERROR;
  }

  // Decode the user configuration from RX_Buffer into loadedConfig struct
  if (DecodeUserConfiguration(RX_Buffer, data_length, &loadedConfig) !=
      USERCONFIG_OK) {
    // Return an error if decoding fails
    return USERCONFIG_DECODE_ERROR;
  }

  return USERCONFIG_OK;  // Return success if decoding is successful
#endif  // TEST_USER_CONFIG
}

// Get a reference to the loaded user configuration data in RAM.
const UserConfiguration *UserConfigGet(void) {
#ifdef TEST_USER_CONFIG
  return &testConfig;
#else
  return &loadedConfig;
#endif  // TEST_USER_CONFIG
}

UserConfigStatus UserConfigSave(const UserConfiguration *config) {
  if (config == NULL) {
    return USERCONFIG_NULL_CONFIG;
  }

  uint8_t encoded_data[UserConfiguration_size];
  size_t encoded_length = EncodeUserConfiguration(config, encoded_data);
  if (encoded_length == -1) {
    return USERCONFIG_ENCODE_ERROR;
  }

  // Write the length of the encoded data to FRAM
  uint8_t length_buf[2] = {(encoded_length >> 8) & 0xFF, encoded_length & 0xFF};
  UserConfigStatus status =
      UserConfig_WriteToFRAM(USER_CONFIG_LEN_ADDR, length_buf, 2);
  if (status != USERCONFIG_OK) {
    return status;
  }

  // Write the encoded data to FRAM
  status = UserConfig_WriteToFRAM(USER_CONFIG_START_ADDRESS, encoded_data,
                                  encoded_length);
  if (status != USERCONFIG_OK) {
    return status;
  }

  return USERCONFIG_OK;
}

void UserConfigPrintAny(const UserConfiguration *config) {
  // Print each member of the UserConfiguration
  APP_PRINTF("Logger ID: %u\r\n", config->logger_id);
  APP_PRINTF("Cell ID: %u\r\n", config->cell_id);

  if (config->Upload_method == 0) {
    APP_PRINTF("Upload Method: %u \"LoRa\"\r\n", config->Upload_method);
  } else {
    APP_PRINTF("Upload Method: %u \"WiFi\"\r\n", config->Upload_method);
  }

  APP_PRINTF("Upload Interval: %u\r\n", config->Upload_interval);

  for (int i = 0; i < config->enabled_sensors_count; i++) {
    const char *sensor_name = NULL;
    switch (config->enabled_sensors[i]) {
      case EnabledSensor_Voltage:
        sensor_name = "Voltage";
        break;
      case EnabledSensor_Current:
        sensor_name = "Current";
        break;
      case EnabledSensor_Teros12:
        sensor_name = "Teros12";
        break;
      case EnabledSensor_Teros21:
        sensor_name = "Teros21";
        break;
      case EnabledSensor_BME280:
        sensor_name = "BME280";
        break;
      case EnabledSensor_Phytos31:
        sensor_name = "Phytos31";
        break;
      case EnabledSensor_SEN0308:
        sensor_name = "SEN0308";
        break;
      case EnabledSensor_SEN0257:
        sensor_name = "SEN0257";
        break;
      case EnabledSensor_YFS210C:
        sensor_name = "YFS210C";
        break;
      case EnabledSensor_PCAP02:
        sensor_name = "PCAP02";
        break;
      default:
        sensor_name = "Unknown Sensor";
    }
    APP_PRINTF("Enabled Sensor %d: %s\r\n", i + 1, sensor_name);
  }

  char float_str[100];

  snprintf(float_str, sizeof(float_str), "%e", config->Voltage_Slope);
  APP_PRINTF("Calibration V Slope: %s\r\n", float_str);

  snprintf(float_str, sizeof(float_str), "%e", config->Voltage_Offset);
  APP_PRINTF("Calibration V Offset: %s\r\n", float_str);

  snprintf(float_str, sizeof(float_str), "%e", config->Current_Slope);
  APP_PRINTF("Calibration I Slope: %s\r\n", float_str);

  snprintf(float_str, sizeof(float_str), "%e", config->Current_Offset);
  APP_PRINTF("Calibration I Offset: %s\r\n", float_str);

  APP_PRINTF("WiFi SSID: %s\r\n", config->WiFi_SSID);

  APP_PRINTF("WiFi Password: %s\r\n", config->WiFi_Password);

  APP_PRINTF("API Endpoint URL: %s\r\n", config->API_Endpoint_URL);

  APP_PRINTF("API Port: %u\r\n", config->API_Endpoint_Port);
}

void UserConfigPrint(void) {
  const UserConfiguration *config = UserConfigGet();

  UserConfigPrintAny(config);
}

UserConfigStatus UserConfigClear(void) {
  FramStatus status = FRAM_OK;
  const uint8_t length[2] = {};
  const uint8_t empty[UserConfiguration_size] = {};
  status = FramWrite(USER_CONFIG_LEN_ADDR, length, sizeof(length));
  if (status != FRAM_OK) {
    return USERCONFIG_FRAM_ERROR;
  }
  status = FramWrite(USER_CONFIG_START_ADDRESS, empty, sizeof(empty));
  if (status != FRAM_OK) {
    return USERCONFIG_FRAM_ERROR;
  }
  return USERCONFIG_OK;
}
