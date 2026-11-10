#include "modbus.h"


ModbusStatus modbus_read_coils(uint8_t id, uint16_t addr, uint16_t count,
                               uint8_t *data, );

ModbusStatus modbus_read_holding_registers(uint8_t id, uint16_t addr,
                                           uint16_t count, uint16_t *data) {}

ModbusStatus modbus_write_coil(uint8_t id, uint16_t addr, bool value) {}

ModbusStatus modbus_write_register(uint8_t id, uint16_t addr, uint16_t value) {}

bool modbus_transaction(uint8_t *tx, uint16_t tx_len, uint8_t *rx,
                        uint16_t rx_len) {
  HAL_StatusTypeDef ret;
  ret = HAL_UART_Transmit(&hlpuart1, (const uint8_t *)tx, tx_len,
                          1000);
  HAL_GPIO_WritePin(SDI12_DIR_GPIO_Port, SDI12_DIR_Pin,
                    GPIO_PIN_SET);  // Set to RX mode
  if (ret != HAL_OK) {
    return 1;
  }

  HAL_StatusTypeDef ret;
  HAL_GPIO_WritePin(SDI12_DIR_GPIO_Port, SDI12_DIR_Pin,
                    GPIO_PIN_SET);  // Set to RX mode
  __HAL_UART_FLUSH_DRREGISTER(&hlpuart1);
  ret =
      HAL_UART_Receive(&hlpuart1, (uint8_t *)rx, rx_len, 1000);
  if (ret == HAL_OK) {
    return 1;
  } else if (ret == HAL_TIMEOUT) {
    return 0;
  } else {
    return 0;
  }

}