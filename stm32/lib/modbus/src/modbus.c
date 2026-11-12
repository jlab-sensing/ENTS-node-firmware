
#include "modbus.h"

uint16_t crc16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];

    for (int j = 0; j < 8; j++) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}

static bool ModbusCheckResponse(uint8_t *rx, uint16_t len) {
  if (len < 5) return false;

  uint16_t crc = crc16(rx, len - 2);

  return rx[len - 2] == (crc & 0xFF) && rx[len - 1] == (crc >> 8);
}

ModbusStatus ModbusReadCoil(uint8_t id, uint16_t addr, uint16_t count,
                            uint8_t *data) {
  if (!count || count > 2000) return MODBUS_ERR;

  uint8_t tx[8];
  uint8_t rx[5 + (count + 7) / 8];

  tx[0] = id;
  tx[1] = 0x01;
  tx[2] = addr >> 8;
  tx[3] = addr & 0xFF;
  tx[4] = count >> 8;
  tx[5] = count & 0xFF;

  uint16_t crc = crc16(tx, 6);
  tx[6] = crc & 0xFF;
  tx[7] = crc >> 8;

  ModbusStatus status = ModbusTransaction(tx, 8, rx, sizeof(rx));
  if (status != MODBUS_OK) return status;

  if (rx[0] != id || rx[1] != 0x01 || rx[2] != (count + 7) / 8 ||
      !ModbusCheckResponse(rx, sizeof(rx)))
    return MODBUS_ERR;

  for (int i = 0; i < count; i++) {
    data[i] = (rx[3 + i / 8] >> (i % 8)) & 1;
  }

  return MODBUS_OK;
}

ModbusStatus ModbusReadRegister(uint8_t id, uint16_t addr, uint16_t count,
                                uint16_t *data) {
  if (!count || count > 125) return MODBUS_ERR;

  uint8_t tx[8];
  uint8_t rx[5 + count * 2];

  tx[0] = id;
  tx[1] = 0x03;
  tx[2] = addr >> 8;
  tx[3] = addr & 0xFF;
  tx[4] = count >> 8;
  tx[5] = count & 0xFF;

  uint16_t crc = crc16(tx, 6);
  tx[6] = crc & 0xFF;
  tx[7] = crc >> 8;

  ModbusStatus status = ModbusTransaction(tx, 8, rx, sizeof(rx));
  if (status != MODBUS_OK) return status;

  if (rx[0] != id || rx[1] != 0x03 || rx[2] != count * 2 ||
      !ModbusCheckResponse(rx, sizeof(rx)))
    return MODBUS_ERR;

  for (int i = 0; i < count; i++) {
    data[i] = (rx[3 + i * 2] << 8) | rx[4 + i * 2];
  }

  return MODBUS_OK;
}

ModbusStatus ModbusWriteCoil(uint8_t id, uint16_t addr, bool value) {
  uint8_t tx[8];
  uint8_t rx[8];

  tx[0] = id;
  tx[1] = 0x05;
  tx[2] = addr >> 8;
  tx[3] = addr & 0xFF;
  tx[4] = value ? 0xFF : 0x00;
  tx[5] = 0x00;

  uint16_t crc = crc16(tx, 6);
  tx[6] = crc & 0xFF;
  tx[7] = crc >> 8;

  ModbusStatus status = ModbusTransaction(tx, 8, rx, 8);
  if (status != MODBUS_OK) return status;

  if (rx[0] != id || rx[1] != 0x05 || !ModbusCheckResponse(rx, 8))
    return MODBUS_ERR;

  for (int i = 0; i < 6; i++) {
    if (rx[i] != tx[i]) return MODBUS_ERR;
  }

  return MODBUS_OK;
}

ModbusStatus ModbusWriteRegister(uint8_t id, uint16_t addr, uint16_t value) {
  uint8_t tx[8];
  uint8_t rx[8];

  tx[0] = id;
  tx[1] = 0x06;
  tx[2] = addr >> 8;
  tx[3] = addr & 0xFF;
  tx[4] = value >> 8;
  tx[5] = value & 0xFF;

  uint16_t crc = crc16(tx, 6);
  tx[6] = crc & 0xFF;
  tx[7] = crc >> 8;

  ModbusStatus status = ModbusTransaction(tx, 8, rx, 8);
  if (status != MODBUS_OK) return status;

  if (rx[0] != id || rx[1] != 0x06 || !ModbusCheckResponse(rx, 8))
    return MODBUS_ERR;

  for (int i = 0; i < 6; i++) {
    if (rx[i] != tx[i]) return MODBUS_ERR;
  }

  return MODBUS_OK;
}

ModbusStatus ModbusTransaction(uint8_t *tx, uint16_t tx_len, uint8_t *rx,
                               uint16_t rx_len) {
  HAL_StatusTypeDef ret;

  HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_PIN, GPIO_PIN_SET);

  ret = HAL_UART_Transmit(&hlpuart1, tx, tx_len, 1000);

  if (ret == HAL_OK) {
    while (__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_TC) == RESET);
  }

  HAL_GPIO_WritePin(MODBUS_DE_GPIO_Port, MODBUS_DE_PIN, GPIO_PIN_RESET);

  if (ret != HAL_OK) return MODBUS_ERR;

  ret = HAL_UART_Receive(&hlpuart1, rx, rx_len, 1000);

  if (ret == HAL_OK) return MODBUS_OK;
  if (ret == HAL_TIMEOUT) return MODBUS_TIM;

  return MODBUS_ERR;
}
