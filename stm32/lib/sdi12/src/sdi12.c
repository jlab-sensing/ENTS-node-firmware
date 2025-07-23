/**
 ******************************************************************************
 * Copyright 2024 jLab
 * @file    sdi12.c
 * @author  Stephen Taylor
 * @brief   This file provides a library to communicate between a STM32 MC
 *          and a TEROS-12 via SDI-12.
 *          https://www.sdi-12.org/
 *
 * @date    4/1/2024
 *
 ******************************************************************************
 **/

#include "sdi12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint16_t REQUEST_MEASURMENT_RESPONSE_SIZE = 7;
static const uint16_t SERVICE_REQUEST_SIZE = 3;
static const uint16_t MEASURMENT_DATA_SIZE = 30;
static const uint16_t SEND_COMMAND_TIMEOUT = 1000;

/* Helper function to parse the sensor's response to a get measurment command*/
SDI12Status ParseMeasurementResponse(const char *responseBuffer, char addr,
                                     SDI12_Measure_TypeDef *measurement_info);

/* Helper function to parse a sensor's service request */
SDI12Status ParseServiceRequest(const char *requestBuffer, char addr);

void SDI12WakeSensors(void) {
  HAL_LIN_SendBreak(&huart2);                            // Send a break
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);  // Send the marking
  // HAL_Delay(20); // Need an extra 10ms to account for the fact
  // that HAL_LIN_SendBreak is nonblocking
  for (int i = 0; i <= 40000; i++) {
    // Figure out a way to do this delay with the low-power timer
    asm("nop");
  }
}

SDI12Status SDI12SendCommand(const char *command, uint8_t size) {
  HAL_StatusTypeDef ret;
  SDI12WakeSensors();
  ret = HAL_UART_Transmit(&huart2, (const uint8_t *)command, size,
                          SEND_COMMAND_TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);  // Set to RX mode
  if (ret == HAL_OK) {
    return SDI12_OK;
  } else {
    return SDI12_ERROR;
  }
}

SDI12Status SDI12ReadData(char *buffer, uint16_t bufferSize,
                          uint16_t timeoutMillis) {
  HAL_StatusTypeDef ret;
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);  // Set to RX mode
  __HAL_UART_FLUSH_DRREGISTER(&huart2);
  ret = HAL_UART_Receive(&huart2, (uint8_t *)buffer, bufferSize, timeoutMillis);
  if (ret == HAL_OK) {
    return SDI12_OK;
  } else if (ret == HAL_TIMEOUT) {
    return SDI12_TIMEOUT_ON_READ;
  } else {
    return SDI12_ERROR;
  }
}

SDI12Status ParseMeasurementResponse(const char *responseBuffer, char addr,
                                     SDI12_Measure_TypeDef *measurement_info) {
  sscanf(responseBuffer, "%1c%3hu%1hhu", &(measurement_info->Address),
         &(measurement_info->Time), &(measurement_info->NumValues));
  // Parse the response and populate the structure
  // Check if the response address matches the expected address
  if (measurement_info->Address == addr) {
    return SDI12_OK;  // Return success
  } else {
    return SDI12_PARSING_ERROR;  // Return error - Address mismatch
  }
}

SDI12Status ParseServiceRequest(const char *requestBuffer, char addr) {
  char expectedResponse[12];
  // Construct the expected response ("a\r\n")
  snprintf(expectedResponse, sizeof(expectedResponse), "%c\r\n", addr);
  // Parse the service request
  // Check if the response matches the expected Active Acknowledge response
  if (memcmp(requestBuffer, expectedResponse, 3) == 0) {
    return SDI12_OK;
  } else {
    return SDI12_PARSING_ERROR;
  }
}

SDI12Status SDI12GetMeasurment(uint8_t addr,
                               SDI12_Measure_TypeDef *measurment_info,
                               char *measurment_data, uint16_t timeoutMillis) {
  // Command to request measurement ("0!\r\n" for example)
  char reqMeas[4];

  // Command for device to send the data
  char sendData[5];

  SDI12Status ret;

  //
  // Sends the measurement request and recieves the immediate response
  //

  // Construct a command to request a measurment
  uint8_t size = snprintf(reqMeas, sizeof(reqMeas), "%cM!", addr);

  char meas_resp_buffer[REQUEST_MEASURMENT_RESPONSE_SIZE];
  // Request a measurment and read the response
  SDI12SendCommand(reqMeas, size);
  ret = SDI12ReadData(meas_resp_buffer, REQUEST_MEASURMENT_RESPONSE_SIZE,
                      timeoutMillis);
  if (ret != SDI12_OK) {
    return ret;
  }

  // Check if the addresses match from the response above.
  // The response from a teros is the same every
  // time so we're going to leave it for now
  ret = ParseMeasurementResponse(meas_resp_buffer, addr, measurment_info);

  if (ret != SDI12_OK) {
    return ret;
  }

  // Construct a command to send the data
  size = snprintf(sendData, sizeof(sendData), "%cD0!", addr);

  // optimization
  if (measurment_info->Time == 0) {  // If data is ready now
    SDI12SendCommand(sendData, size);
    // hard coded for a teros measurment response size
    ret = SDI12ReadData(measurment_data, MEASURMENT_DATA_SIZE, timeoutMillis);
    return ret;
  }

  //
  // Blocks until the sensor sends the service request, noted by a "a\r\n"
  //

  char service_request_resp[SERVICE_REQUEST_SIZE];

  // Read the service request, wait's for one to arrive
  ret =
      SDI12ReadData(service_request_resp, SERVICE_REQUEST_SIZE, timeoutMillis);
  if (ret != SDI12_OK) {
    return ret;
  }

  // make sure the service request is correct
  ret = ParseServiceRequest(service_request_resp, measurment_info->Address);
  if (ret != SDI12_OK) {
    return ret;
  }

  //
  // Send request for data and receive data from device
  //

  SDI12SendCommand(sendData, size);
  ret = SDI12ReadData(measurment_data, MEASURMENT_DATA_SIZE, timeoutMillis);

  if ((ret == SDI12_OK) || (ret == SDI12_TIMEOUT_ON_READ)) {
    // remove trailing characters after \r\n
    char *end = strstr(measurment_data, "\r\n");

    if (!end) {
      return SDI12_ERROR;
    }

    *end = '\0';

    return SDI12_OK;
  }
  // IT IS VERY IMPORTANT THAT THE AMOUNT OF DATA EXPECTED
  // TO BE READ (bufferSize) MATCH WHAT IS SENT EXACTLY.
  // Or maybe slightly smaller, but any greater causes the program to hang
  return ret;
}

SDI12Status SDI12GetAddress(char *addr, uint16_t timeoutMillis) {
  SDI12Status ret = SDI12_OK;

  // query the sensor
  char addr_query_cmd[] = "?!";
  ret = SDI12SendCommand(addr_query_cmd, 2);
  if (ret != SDI12_OK) {
    return ret;
  }

  // get response
  char resp_buffer[4] = {};
  ret = SDI12ReadData(resp_buffer, 4, timeoutMillis);
  if (ret == SDI12_OK || ret == SDI12_TIMEOUT_ON_READ) {
    ret = SDI12_OK;
  } else {
    return ret;
  }

  // parse the response, return error if parsing fails
  int params = 0;
  params = sscanf(resp_buffer, "%1c\r\n", addr);
  if (params != 1) {
    return SDI12_PARSING_ERROR;
  }

  return ret;
}
