#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "stdbool.h"

typedef enum {
  MODBUS_OK = 0,
  MODBUS_ERR = -1,
} ModbusStatus;

ModbusStatus modbus_read_coils(uint8_t id, uint16_t addr,
                       uint16_t count, uint8_t *data);

ModbusStatus modbus_read_holding_registers(uint8_t id, uint16_t addr,
                                   uint16_t count, uint16_t *data);

ModbusStatus modbus_write_coil(uint8_t id, uint16_t addr, bool value);

ModbusStatus modbus_write_register(uint8_t id, uint16_t addr, uint16_t value);

bool modbus_transaction(uint8_t *tx, uint16_t tx_len,
                        uint8_t *rx, uint16_t rx_len);