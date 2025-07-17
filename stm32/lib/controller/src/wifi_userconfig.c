#include "userConfig.h"
#include "communication.h"
#include "transcoder.h"
#include "controller/wifi_userconfig.h"
#include "stm32_adv_trace.h"

// Static variable for I2C timeout (same as WiFi)
static unsigned int g_controller_i2c_timeout = 10000;

UserConfigStatus ControllerUserConfigRequest(void) {
    Buffer *tx = ControllerTx();
    Buffer *rx = ControllerRx();

    // Clear buffers
    memset(tx->data, 0, tx->size);
    memset(rx->data, 0, rx->size);
    tx->len = 0;
    rx->len = 0;

    // Create request
    UserConfigCommand request = {0};
    request.type = UserConfigCommand_RequestType_REQUEST_CONFIG;
    request.has_config_data = false;

    // Encode and send request
    tx->len = EncodeUserConfigCommand(request.type, NULL, tx->data, tx->size);
    if (tx->len == 0) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config request\r\n");
        return USERCONFIG_ENCODE_ERROR;
    }

    ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
    if (status != CONTROLLER_SUCCESS) {
        APP_LOG(TS_OFF, VLEVEL_M, "Config request failed: %d\r\n", status);
        return USERCONFIG_COMM_ERROR;
    }

    // Check if we got a response
    if (rx->len == 0) {
        APP_LOG(TS_OFF, VLEVEL_M, "Empty config response\r\n");
        return USERCONFIG_NO_RESPONSE;
    }

    // Decode the response
    Esp32Command cmd = DecodeEsp32Command(rx->data, rx->len);
    if (cmd.which_command != Esp32Command_user_config_command_tag) {
        APP_LOG(TS_OFF, VLEVEL_M, "Invalid response type\r\n");
        return USERCONFIG_INVALID_RESPONSE;
    }

    if (cmd.command.user_config_command.type == UserConfigCommand_RequestType_RESPONSE_CONFIG && 
        cmd.command.user_config_command.has_config_data) {
        
        const UserConfiguration *config = &cmd.command.user_config_command.config_data;
        
        // Check if config is all zeros (uninitialized)
        if (isConfigEmpty(config)) {
            APP_LOG(TS_OFF, VLEVEL_M, "Received empty config from ESP32\r\n");
            return USERCONFIG_EMPTY_CONFIG;
        }
        
        // Print received config
        APP_LOG(TS_OFF, VLEVEL_M, "-----------------Received configuration from ESP32-----------------:\r\n");
        printUserConfig(config);
        
        return USERCONFIG_OK;
    }
    
    APP_LOG(TS_OFF, VLEVEL_M, "Invalid config response format\r\n");
    return USERCONFIG_INVALID_RESPONSE;
}

UserConfigStatus ControllerUserConfigSend(void) {
    // Load config from FRAM
    UserConfigStatus fram_status = UserConfigLoad();
    if (fram_status != USERCONFIG_OK) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to load config from FRAM: %d\r\n", fram_status);
        return fram_status;
    }

    const UserConfiguration *config = UserConfigGet();
    if (config == NULL) {
        APP_LOG(TS_OFF, VLEVEL_M, "Null config received from FRAM\r\n");
        return USERCONFIG_NULL_CONFIG;
    }

    // Print config being sent
    APP_LOG(TS_OFF, VLEVEL_M, "-----------------Sending configuration to ESP32-----------------:\r\n");
    printUserConfig(config);

    Buffer *tx = ControllerTx();
    Buffer *rx = ControllerRx();

    // Clear buffers
    memset(tx->data, 0, tx->size);
    memset(rx->data, 0, rx->size);
    tx->len = 0;
    rx->len = 0;

    // Prepare response
    UserConfigCommand response = {0};
    response.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
    response.has_config_data = true;
    memcpy(&response.config_data, config, sizeof(UserConfiguration));

    // Encode and send
    tx->len = EncodeUserConfigCommand(response.type, &response.config_data, tx->data, tx->size);
    if (tx->len == 0) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config response\r\n");
        return USERCONFIG_ENCODE_ERROR;
    }

    ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
    if (status != CONTROLLER_SUCCESS) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config: %d\r\n", status);
        return USERCONFIG_COMM_ERROR;
    }

    APP_LOG(TS_OFF, VLEVEL_M, "Configuration successfully sent to ESP32\r\n");
    return USERCONFIG_OK;
}

bool isConfigEmpty(const UserConfiguration *config) {
    // Check if logger_id & cell_id fields are zero/default
    return (config->logger_id == 0 &&
            config->cell_id == 0 );
}

void printUserConfig(const UserConfiguration *config) {
    HAL_Delay(10);
    // Print each member of the UserConfiguration
    sprintf(uart_buf, "Logger ID: %lu\r\n", config->logger_id);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "Cell ID: %lu\r\n", config->cell_id);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    if (config->Upload_method == 0) {
      sprintf(uart_buf, "Upload Method: %u \"LoRa\"\r\n", config->Upload_method);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);
    } else {
      sprintf(uart_buf, "Upload Method: %u \"WiFi\"\r\n", config->Upload_method);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);
    }

    HAL_Delay(10);

    sprintf(uart_buf, "Upload Interval: %lu\r\n", config->Upload_interval);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    for (int i = 0; i < config->enabled_sensors_count; i++) {
      const char *sensor_name;
      switch (config->enabled_sensors[i]) {
        case 0:
          sensor_name = "Voltage";
          break;
        case 1:
          sensor_name = "Current";
          break;
        case 2:
          sensor_name = "Teros12";
          break;
        case 3:
          sensor_name = "Teros21";
          break;
        case 4:
          sensor_name = "BME280";
          break;
      }
      sprintf(uart_buf, "Enabled Sensor %d: %s\r\n", i + 1, sensor_name);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);
    }

    HAL_Delay(10);

    sprintf(uart_buf, "Calibration V Slope: %f\r\n", config->Voltage_Slope);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "Calibration V Offset: %f\r\n", config->Voltage_Offset);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "Calibration I Slope: %f\r\n", config->Current_Slope);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "Calibration I Offset: %f\r\n", config->Current_Offset);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "WiFi SSID: %s\r\n", config->WiFi_SSID);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(10);

    sprintf(uart_buf, "WiFi Password: %s\r\n", config->WiFi_Password);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(20);
    
    sprintf(uart_buf, "API Endpoint URL: %s\r\n", config->API_Endpoint_URL);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf), HAL_MAX_DELAY);

    HAL_Delay(20);
    APP_LOG(TS_OFF, VLEVEL_M, "--------------------------------------------\r\n");
    HAL_Delay(10);
}