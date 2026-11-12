#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "main.h"
#include "tim.h"
#include "usart.h"

typedef enum {
  MODBUS_OK = 0,
  MODBUS_ERR = -1,
  MODBUS_TIM = -2,
} ModbusStatus;

ModbusStatus ModbusReadCoil(uint8_t id, uint16_t addr, uint16_t count,
                            uint8_t *data);

ModbusStatus ModbusReadRegister(uint8_t id, uint16_t addr, uint16_t count,
                                uint16_t *data);

ModbusStatus ModbusWriteCoil(uint8_t id, uint16_t addr, bool value);

ModbusStatus ModubsWriteRegister(uint8_t id, uint16_t addr, uint16_t value);

ModbusStatus ModbusTransaction(uint8_t *tx, uint16_t tx_len, uint8_t *rx,
                               uint16_t rx_len);