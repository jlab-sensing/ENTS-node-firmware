#include "pcap02.h"

#include "board.h"
#include "i2c.h"
#include "userConfig.h"

// Private Globals
static HAL_StatusTypeDef ret = HAL_OK;
static volatile uint16_t dev_addr = PCAP02_I2C_ADDRESS;

volatile uint32_t INTN_Counter = 0;
volatile uint8_t INTN_State = GPIO_PIN_SET;

// Private Function Definitions
uint32_t test_sram_write_byte(uint8_t txData, uint16_t location);
uint32_t test_sram_write_memory_access(uint8_t txData[], uint8_t rxData[],
                                       uint16_t location, uint32_t length);

void pcap02_init(void) {
  pcap02_read_register_status_t status_register;
  uint8_t runbit;

  /* runbit = 0x00;
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_CONFIG_WRITE, PCAP02_CONFIG_REGISTERS_RUNBIT_REG,
      &runbit,
      1);  // Disable reg 77 runbit as recommended before any memory access.

  APP_LOG(TS_OFF, VLEVEL_H,
          "OPCODE POR: Writing opcode 0x%02X to device 0x%02X\r\n",
          PCAP02_OPCODE_POR, dev_addr);
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_POR);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(500);

  if (test_sram_write_byte(0xA5, 0x047)) {
    APP_LOG(TS_OFF, VLEVEL_H, "Failed to write byte to pcap02 sram.\r\n");
    Error_Handler();
  }

  uint8_t txData[100];
  for (uint16_t i = 0; i < sizeof(txData); i++) {
    txData[i] = i;
    APP_LOG(TS_OFF, VLEVEL_H, "%d ", txData[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_H, "\r\n");
  uint8_t rxData[sizeof(txData)];
  if (test_sram_write_memory_access(txData, rxData, 0, sizeof(txData))) {
    APP_LOG(TS_OFF, VLEVEL_H,
            "Failed to write memory access to pcap02 sram.\r\n");
    Error_Handler();
  }

  APP_LOG(TS_OFF, VLEVEL_H, "\r\nTests concluded.\r\n\r\n");

  // POR + INIT
  APP_LOG(TS_OFF, VLEVEL_H, "Power On Reset\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_POR);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(500);
  APP_LOG(TS_OFF, VLEVEL_H, "Initialize\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_INITIALIZE);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(10); */

  // 1. Set configuration register 77 to RUNBIT = 0
  APP_LOG(TS_OFF, VLEVEL_H, "Setting runbit to 0.\r\n");
  runbit = 0x00;
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_CONFIG_WRITE, PCAP02_CONFIG_REGISTERS_RUNBIT_REG,
      &runbit,
      1);  // Disable reg 77 runbit as recommended before any memory access.
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  // 2. Write standard configuration register values for registers 0 - 76
  APP_LOG(TS_OFF, VLEVEL_H,
          "Writing to all (non-runbit) config registers.\r\n");
  ret = I2C_Config_Access(dev_addr, PCAP02_OPCODE_CONFIG_WRITE, 0,
                          pcap02_standard_config_registers,
                          PCAP02_CONFIG_REGISTERS_LENGTH - 1);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  // 3. Write standard firmware (and version information) to SRAM
  APP_LOG(TS_OFF, VLEVEL_H, "Writing standard firmware to SRAM.\r\n");
  pcap02_sram_write_firmware(pcap02_standard_firmware,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_OFFSET +
                                 PCAP02_STANDARD_FIRMWARE_MAIN_OFFSET,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_LENGTH);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_H, "Writing standard firmware version to SRAM.\r\n");
  pcap02_sram_write_firmware(pcap02_standard_firmware_version,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_OFFSET +
                                 PCAP02_STANDARD_FIRMWARE_VERSION_OFFSET,
                             PCAP02_STANDARD_FIRMWARE_VERSION_LENGTH);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  // APP_LOG(TS_OFF, VLEVEL_H,
  //         "Reading status register. (Before runbit=1 and final "
  //         "initialization)\r\n");
  // ret = I2C_Config_Access(
  //     dev_addr, PCAP02_OPCODE_RESULT_READ,
  //     PCAP02_READ_REGISTERS_STATUS_OFFSET, status_register.byte,
  //     PCAP02_READ_REGISTERS_REG_LENGTH_BYTES);  // Read status register
  // if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
  // pcap02_print_status_register(status_register);

  // 4. Set configuration register 77 to RUNBIT = 1
  APP_LOG(TS_OFF, VLEVEL_H, "Setting runbit to 1.\r\n");
  runbit = 0x01;
  ret = I2C_Config_Access(dev_addr, PCAP02_OPCODE_CONFIG_WRITE,
                          PCAP02_CONFIG_REGISTERS_RUNBIT_REG, &runbit, 1);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  // 5. Send partial reset (Initialize)
  APP_LOG(TS_OFF, VLEVEL_H, "Initialize.\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_INITIALIZE);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
}

void pcap02_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : INTN_Pin */
  GPIO_InitStruct.Pin = PCAP02_INTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PCAP02_INTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(PCAP02_INTN_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PCAP02_INTN_EXTI_IRQn);
}

void pcap02_start_conversion(void) {
  // 6. Start measurement (CDC Start conversion)
  APP_LOG(TS_OFF, VLEVEL_H, "CDC Start Conversion.\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_CDC_START_CONVERSION);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);

  // 7. Read status registers 0x24 25 26 (decimal 36 37 38)
  // APP_LOG(
  //     TS_OFF, VLEVEL_H,
  //     "Reading status register. (After runbit=1 and final
  //     initialization)\r\n");
  // ret = I2C_Config_Access(
  //     dev_addr, PCAP02_OPCODE_RESULT_READ,
  //     PCAP02_READ_REGISTERS_STATUS_OFFSET, status_register.byte,
  //     PCAP02_READ_REGISTERS_REG_LENGTH_BYTES);  // Read status register
  // if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\tret (HAL) %d\r\n", ret);
  // pcap02_print_status_register(status_register);
}

/**
 * @brief  Write firmware to SRAM.
 * @param  firmware (uint8_t*) Pointer to byte array containing firmware.
 * @param  offset_bytes (uint16_t) Offset in bytes from SRAM address 0 to begin
 * writing at.
 * @param  length_bytes (uint16_t) Length in bytes to copy from `firmware` to
 * SRAM.
 * @retval (uint16_t) Number of byte-errors detected during post-write
 * verification of firmware.
 */
uint16_t pcap02_sram_write_firmware(uint8_t *firmware, uint16_t offset_bytes,
                                    uint16_t length_bytes) {
  // uint8_t txDataLong[4096];
  uint8_t rxDataLong[4096] = {0};

  APP_LOG(TS_OFF, VLEVEL_H, "\r\npcap02_sram_write_firmware START\r\n");

  APP_LOG(TS_OFF, VLEVEL_H, "\tFirmware to write (hex):");
  APP_LOG(TS_OFF, VLEVEL_H,
          "\r\n\t\t         +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F");
  if (offset_bytes % 16 != 0) {
    APP_LOG(TS_OFF, VLEVEL_H,
            "\r\n\t\t[0x%03X]: ", offset_bytes - (offset_bytes % 16));
    for (uint16_t i = 0; i < (offset_bytes % 16); i++) {
      APP_LOG(TS_OFF, VLEVEL_H, "__ ");
    }
  }
  for (uint16_t i = 0; i < length_bytes; i++) {
    if (((i + (offset_bytes % 16)) % 16) == 0) {
      APP_LOG(TS_OFF, VLEVEL_H,
              "\r\n\t\t[0x%03X]: ", ((i + (offset_bytes % 16)) / 16) << 4);
    }
    APP_LOG(TS_OFF, VLEVEL_H, "%02X ", firmware[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_H, "\r\n");

  APP_LOG(TS_OFF, VLEVEL_H, "\tWriting %d bytes to SRAM.\r\n", length_bytes);
  ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_WRITE, offset_bytes,
                          firmware, length_bytes);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\t\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_H, "\tReading %d bytes from SRAM.\r\n", length_bytes);
  ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_READ, offset_bytes,
                          rxDataLong, length_bytes);
  if (ret) APP_LOG(TS_OFF, VLEVEL_H, "\t\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_H, "\t\t(Received firmware omitted.)\r\n");
  // for (uint16_t i = 0; i < sizeof(rxDataLong); i++)
  // {
  //   APP_LOG(TS_OFF, VLEVEL_H, "%d ", rxDataLong[i]);
  // }

  APP_LOG(TS_OFF, VLEVEL_H, "\tVerify transmitted and received firmware.\r\n");
  uint16_t error_count = 0;
  for (uint16_t i = 0; i < length_bytes; i++) {
    if (firmware[i] != rxDataLong[i]) {
      error_count += 1;
    }
  }
  APP_LOG(TS_OFF, VLEVEL_H, "\t\tErrors: %d --> %s\r\n", error_count,
          (error_count) ? ("FAIL") : ("PASS"));

  APP_LOG(TS_OFF, VLEVEL_H, "\r\npcap02_sram_write_firmware END\r\n");

  return error_count;
}

/**
 * @brief  Print contents of the 24-bit status register.
 * @note   This function does not send any commands to read from the PCAP02.
 *         The input `status` must first be loaded with the contents of the
 * status register.
 * @param  status (pcap02_read_register_status_t) Union with bit fields for the
 * status register.
 * @retval (void)
 */
void pcap02_print_status_register(pcap02_read_register_status_t status) {
  APP_LOG(TS_OFF, VLEVEL_H, "0x%02X%02X%02X\r\n", status.byte[0],
          status.byte[1], status.byte[2]);

  APP_LOG(TS_OFF, VLEVEL_H,
          "STATUS_0:\r\n"
          "\t.0 RunBit: %d\r\n"
          "\t.1 CDC_active: %d\r\n"
          "\t.2 RDC_ready: %d\r\n"
          "\t.3 EEPROM_busy: %d\r\n"
          "\t.4 AutoBoot_busy: %d\r\n"
          "\t.5 POR_Flag_SRAM: %d\r\n"
          "\t.6 POR_Flag_Config: %d\r\n"
          "\t.7 POR_Flag_Wdog: %d\r\n",
          status.STATUS_0.RunBit, status.STATUS_0.CDC_active,
          status.STATUS_0.RDC_ready, status.STATUS_0.EEPROM_busy,
          status.STATUS_0.AutoBoot_busy, status.STATUS_0.POR_Flag_SRAM,
          status.STATUS_0.POR_Flag_Config, status.STATUS_0.POR_Flag_Wdog);
  APP_LOG(TS_OFF, VLEVEL_H,
          "STATUS_1:\r\n"
          "\t.0 Comb_Err: %d\r\n"
          "\t.2 Err_Ovfl: %d\r\n"
          "\t.3 Mup_Err: %d\r\n"
          "\t.4 RDC_Err: %d\r\n",
          status.STATUS_1.Comb_Err, status.STATUS_1.Err_Ovfl,
          status.STATUS_1.Mup_Err, status.STATUS_1.RDC_Err);
  APP_LOG(TS_OFF, VLEVEL_H,
          "STATUS_2:\r\n"
          "\t.0 C_PortError0: %d\r\n"
          "\t.1 C_PortError1: %d\r\n"
          "\t.2 C_PortError2: %d\r\n"
          "\t.3 C_PortError3: %d\r\n"
          "\t.4 C_PortError4: %d\r\n"
          "\t.5 C_PortError5: %d\r\n"
          "\t.6 C_PortError6: %d\r\n"
          "\t.7 C_PortError7: %d\r\n",
          status.STATUS_2.C_PortError0, status.STATUS_2.C_PortError1,
          status.STATUS_2.C_PortError2, status.STATUS_2.C_PortError3,
          status.STATUS_2.C_PortError4, status.STATUS_2.C_PortError5,
          status.STATUS_2.C_PortError6, status.STATUS_2.C_PortError7);
}

/******************************************************************************/
/*                            Sweep Device Address                            */
/******************************************************************************/
/**
  * @brief  sweep device address to find the address of connected slave
  * @param  from_addr (7 bit) start at device address
  * @param  to_addr (7 bit) end at devie address
  * @param  *addr_array returns the found device address(es)
  * @retval none
  *
  * @verbatim
  ==============================================================================
                                         ##### How to use this routine #####
  ==============================================================================
        uint8_t slave[1] = 0;
        I2C_Sweep_DevAddr(0, 127, slave);
        dev_addr = slave[0];
  */
void I2C_Sweep_DevAddr(uint8_t from_addr, uint8_t to_addr,
                       uint8_t *addr_array) {
  /* 3 is number of trials, 1ms is timeout */
  uint8_t timeout = 1;
  uint8_t trials = 3;

  for (int check_addr = from_addr; check_addr <= to_addr; check_addr++) {
    /* Checks if target device is ready for communication. */
    if (HAL_I2C_IsDeviceReady(&hi2c2, check_addr, trials, timeout) != HAL_OK) {
      // check_addr++;
      HAL_Delay(1);
      /* Return error */
      // return HAL_ERROR;
    } else {
      addr_array[check_addr] = check_addr;
      // End FOR-Loop
      // break;
    }
  }
}

/******************************************************************************/
/*                            Write one byte Opcode                           */
/******************************************************************************/
/**
 * @brief  Write one byte Opcode.
 * @param  slave (7 Bit) device address
 * @param  one_byte
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef I2C_Write_Opcode(uint8_t slave, uint8_t one_byte) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 1;
  uint8_t i2cTX[1];

  i2cTX[0] = one_byte;

  /* 1. Transmit register address */
  while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 1, timeout) !=
         HAL_OK) {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
      Error_Handler();
    }
  }

  while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

  return HAL_OK;
}

/******************************************************************************/
/*                             Memory Access                            */
/******************************************************************************/
/**
 * @brief  Write one byte.
 * @param  opcode (byte)
 * @param  address (byte)
 * @param  byte (byte)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef I2C_Memory_Access(uint8_t slave, uint8_t opcode,
                                    uint16_t address, uint8_t *byte,
                                    uint16_t size) {
  /* Calculation of max timeout using I2C frequency = 100kHz
   *  ( 1 / 100kHz ) x (2 + size) x (8 bit + ACK bit) = minimum timeout
   *   */
  /* Timeout duration in millisecond [ms] */
  // float savety_factor = 1.2e3;
  // // plus 10% and to get timeout value in [ms] uint32_t timeout = ((1 /
  // 100e3) * (2 + size) * 9) * savety_factor; // 1000;

  // Approximation for the calculation above:
  // uint32_t timeout = 10 * size / 92; // + 0.216;
  uint32_t timeout = 1000;

  if (opcode == PCAP02_OPCODE_SRAM_WRITE) {
    uint8_t i2cTX[size + 2];

    i2cTX[0] = opcode | (uint8_t)(address >> 8);
    i2cTX[1] = (uint8_t)address;

    for (int i = 0; i < size; i++) {
      i2cTX[2 + i] = byte[i];
    }

    // WR
    /* Send WHO_AM_I register address */
    while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, (2 + size),
                                   timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);
  } else if (opcode == PCAP02_OPCODE_SRAM_READ) {
    uint8_t i2cTX[2];
    uint8_t i2cRX[size];

    i2cTX[0] = opcode | (uint8_t)(address >> 8);
    i2cTX[1] = (uint8_t)address;

    // RD
    /* Send WHO_AM_I register address */
    while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 2,
                                   timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

    /* Receieve data in the register */
    while (HAL_I2C_Master_Receive(&hi2c2, (uint16_t)slave, i2cRX, size,
                                  timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

    // copy array
    for (int i = 0; i < size; i++) {
      byte[i] = i2cRX[i];
    }
  } else {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/******************************************************************************/
/*                         Configuration Access                         */
/******************************************************************************/
/**
 * @brief  Write one double word.
 * @param  slave (7 Bit) device address
 * @param  opcode (byte)
 * @param  address (byte)
 * @param  dword (double word)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef I2C_Config_Access(uint8_t slave, uint8_t opcode,
                                    uint8_t address, uint8_t *byte,
                                    uint8_t size) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 100;

  if (opcode == PCAP02_OPCODE_CONFIG_WRITE) {
    uint8_t i2cTX[2 + size];

    i2cTX[0] = opcode;
    i2cTX[1] = address;

    for (int i = 0; i < size; i++) {
      i2cTX[2 + i] = byte[i];
    }
    // WR
    while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, (2 + size),
                                   timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);
  } else if (opcode == PCAP02_OPCODE_RESULT_READ) {
    uint8_t i2cTX[2];
    uint8_t i2cRX[size];

    i2cTX[0] = opcode;
    i2cTX[1] = address;

    // RD
    while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 2,
                                   timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

    while (HAL_I2C_Master_Receive(&hi2c2, (uint16_t)slave, i2cRX, size,
                                  timeout) != HAL_OK) {
      /* Error_Handler() function is called when Timeout error occurs.
         When Acknowledge failure occurs (Slave don't acknowledge it's address)
         Master restarts communication */
      if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
        Error_Handler();
      }
    }
    while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

    // copy array
    for (int i = 0; i < size; i++) {
      byte[i] = i2cRX[i];
    }
  } else {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/******************************************************************************/
/*                         Write one data double word                         */
/******************************************************************************/
/**
 * @brief  Write one double word.
 * @param  slave (7 Bit) device address
 * @param  opcode (byte)
 * @param  address (byte)
 * @param  dword (double word)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef I2C_Write_Dword(uint8_t slave, uint8_t opcode,
                                  uint8_t address, uint32_t dword) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 10;
  uint8_t i2cTX[6];
  uint32_t temp_u32 = 0;

  i2cTX[0] = opcode;
  i2cTX[1] = address;
  temp_u32 = dword;
  i2cTX[5] = temp_u32 >> 24;
  i2cTX[4] = temp_u32 >> 16;
  i2cTX[3] = temp_u32 >> 8;
  i2cTX[2] = temp_u32;

  /* 1. Transmit register address */
  while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 6, timeout) !=
         HAL_OK) {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
      Error_Handler();
    }
  }
  while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

  return HAL_OK;
}

/******************************************************************************/
/*                             Write one data byte                            */
/******************************************************************************/
/**
 * @brief  Write one byte.
 * @param  opcode (byte)
 * @param  address (byte)
 * @param  byte (byte)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef I2C_Write_Byte(uint8_t slave, uint8_t opcode, uint8_t address,
                                 uint8_t byte) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 10;
  uint8_t i2cTX[3];

  i2cTX[0] = opcode;
  i2cTX[1] = address;
  i2cTX[2] = byte;

  /* 1. Transmit register address */
  while (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 3, timeout) !=
         HAL_OK) {
    /* Error_Handler() function is called when Timeout error occurs.
       When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
    if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF) {
      Error_Handler();
    }
  }
  while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);

  return HAL_OK;
}

/******************************************************************************/
/*                              Read double word                              */
/******************************************************************************/
/**
 * @brief  Read double word.
 * @param  opcode (byte)
 * @param  address (byte)
 * @retval 32-bit value
 */
uint32_t I2C_Read_Dword(uint8_t slave, uint8_t rd_opcode, uint8_t address) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 10;
  uint8_t i2cTX[2];
  uint8_t i2cRX[4];
  uint32_t temp_u32 = 0;

  i2cTX[0] = rd_opcode;
  i2cTX[1] = address;

  /* 1. Transmit register address */
  HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 2, timeout);

  /* 2. Read four bytes */
  HAL_I2C_Master_Receive(&hi2c2, (uint16_t)slave, i2cRX, 4, timeout);

  /* Concatenate of bytes (from MSB to LSB) */
  temp_u32 = (i2cRX[0] << 24) + (i2cRX[1] << 16) + (i2cRX[2] << 8) + (i2cRX[3]);

  return temp_u32;
}

/******************************************************************************/
/*                                  Read byte                                 */
/******************************************************************************/
/**
 * @brief  Read byte.
 * @param  opcode (byte)
 * @param  address (byte)
 * @retval 8-bit value
 */
uint8_t I2C_Read_Byte(uint8_t slave, uint8_t rd_opcode, uint8_t address) {
  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 10;
  uint8_t i2cTX[2];
  uint8_t i2cRX[1];
  uint8_t temp_u8 = 0;

  i2cTX[0] = rd_opcode;
  i2cTX[1] = address;

  /* 1. Transmit register address */
  HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 2, timeout);

  /* 2. Read four bytes */
  HAL_I2C_Master_Receive(&hi2c2, (uint16_t)slave, i2cRX, 1, timeout);

  /* Concatenate of bytes (from MSB to LSB) */
  temp_u8 = i2cRX[0];

  return temp_u8;
}

/******************************************************************************/
/*                            Read Result Register                            */
/******************************************************************************/
/**
 * @brief  Read double word.
 * @param  opcode (byte)
 * @param  address (byte)
 * @retval 32-bit value
 */
uint32_t I2C_Read_Result(uint8_t slave, uint8_t rd_opcode, uint8_t address) {
  /* Definition of order, which bit is read first.
   * For example, with PICOCAP #define must be commented out */
  // #define MSB2LSB

  /* Timeout duration in millisecond [ms] */
  uint8_t timeout = 10;
  uint8_t i2cTX[2];
  uint8_t i2cRX[3];
  uint32_t temp_u32 = 0;

  i2cTX[0] = rd_opcode;
  i2cTX[1] = address;

  HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)slave, i2cTX, 2, timeout);
  HAL_I2C_Master_Receive(&hi2c2, (uint16_t)slave, i2cRX, 3, timeout);

  /* 1. Transmit register address */
  //	while(HAL_I2C_Master_Transmit(&hi2c2, slave, i2cTX, 1, timeout) !=
  // HAL_OK) {
  /* Error_Handler() function is called when Timeout error occurs.
     When Acknowledge failure occurs (Slave don't acknowledge it's address)
     Master restarts communication */
  /*		if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF)
                  {
                    Error_Handler();
                  }
          }
          while(HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);
  */
  /* 2. Read four bytes */
  //	while(HAL_I2C_Master_Receive(&hi2c2, slave, i2cRX, 4, timeout) !=
  // HAL_OK) {
  /* Error_Handler() function is called when Timeout error occurs.
     When Acknowledge failure occurs (Slave don't acknowledge it's address)
     Master restarts communication */
/*		if (HAL_I2C_GetError(&hi2c2) != HAL_I2C_ERROR_AF)
                {
                  Error_Handler();
                }
        }
        while(HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY);
*/
#ifdef MSB2LSB
  /* Concatenate of bytes (from MSB to LSB) */
  temp_u32 = (i2cRX[0] << 24) + (i2cRX[1] << 16) + (i2cRX[2] << 8) + (i2cRX[3]);
#else
  /* Concatenate of bytes (from LSB to MSB), e.g. used by PICOCAP */
  // temp_u32 = (i2cRX[3] << 24) + (i2cRX[2] << 16) + (i2cRX[1] << 8) +
  // (i2cRX[0]);
  temp_u32 = (i2cRX[2] << 16) + (i2cRX[1] << 8) + (i2cRX[0]);
#endif

  return temp_u32;
}

// Note: location is a 12-bit address (lower 12 bits only)
uint32_t test_sram_write_byte(uint8_t txData, uint16_t location) {
  uint8_t rxData = 0x00;

  APP_LOG(
      TS_OFF, VLEVEL_M,
      "TEST SRAM RANDOM WRITE: Write 0x%02X at SRAM address 0x%02X to device "
      "0x%02X\r\n",
      txData, location, dev_addr);
  ret = I2C_Write_Byte(dev_addr, PCAP02_OPCODE_SRAM_WRITE, location, txData);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_M,
          "Reading at SRAM address 0x%02X on device 0x%02X\r\n", location,
          dev_addr);
  rxData = I2C_Read_Byte(dev_addr, PCAP02_OPCODE_SRAM_READ, location);
  APP_LOG(TS_OFF, VLEVEL_M, "Received 0x%02X at SRAM address 0x%02X\r\n",
          rxData, location);

  if (txData == rxData) {
    return 0;
  } else {
    return -1;
  }
}

uint32_t test_sram_write_memory_access(uint8_t txData[], uint8_t rxData[],
                                       uint16_t location, uint32_t length) {
  APP_LOG(TS_OFF, VLEVEL_M,
          "\r\nTEST SRAM SEQUENTIAL WRITE: Writing %u bytes to SRAM (at "
          "+0x%03X).\r\n",
          length, location);
  ret =
      I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_WRITE, 0, txData, length);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_M, "\r\nReading %u bytes from SRAM.\r\n", length);
  ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_READ, 0, rxData, length);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  for (uint16_t i = 0; i < length; i++) {
    APP_LOG(TS_OFF, VLEVEL_M, "%d ", rxData[i]);
    if (rxData[i] != txData[i]) {
      return -1;
    }
  }
  return 0;
}

size_t pcap02_measure_capacitance(pcap02_result_t *result) {
  // 8. ‘h40 03 00 00 00; Read Res1, addresses 3, 4, 5. Res1 is expected to be
  //                      in the range of 2,000,000 or ’h2000XX if the two
  //                      capacitors are of same size. Res1 has the format of a
  //                      fixed point number with 3 integer digits and 21
  //                      fractional digits. So, dividing the 2,000,000 by 2^21
  //                      gives a factor of about 1 for the ratio C1/C0.

  // Read result register after INTN = 0
  // Note: Approximately 82 ms or 83 ms between prints.
  if (INTN_State == GPIO_PIN_RESET) {
    INTN_State = GPIO_PIN_SET;
    // APP_LOG(
    //     TS_OFF, VLEVEL_H,
    //     "[reading %lu @ tick %lu] Read Result on RES1 (ratio of C1 /
    //     C0)\r\n", INTN_Counter, HAL_GetTick());
    // MyRawRES0 = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ, 0x00);
    // MyRawRES1 = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ, 0x04);
    result->word = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ,
                                   PCAP02_READ_REGISTERS_RES1_OFFSET);
    // APP_LOG(TS_OFF, VLEVEL_H,
    //         "\tbytes: 0x%02X%02X%02X\r\n\t24-bit: 0x%06X\r\n\tfixed: "
    //         "%01d\r\n\tfractional (raw): %d\r\n\tfloat RATIO: %f\r\n",
    //         RES1.byte[2], RES1.byte[1], RES1.byte[0], RES1.word,
    //         RES1.fixed, RES1.fractional, fixed_to_float(RES1));

    // Post Processing
    // MyRatioRES0 = (float)MyRawRES0 / 134217728; // = 2^27
    // MyRatioRES1 = (float)MyRawRES1 / 134217728; // = 2^27

    // APP_LOG(TS_OFF, VLEVEL_H,"\r\nRES0: raw %ld ratio %f\r\nRES1: raw %ld
    // ratio %f\r\n", MyRawRES0, MyRatioRES0, MyRawRES1, MyRatioRES1);
    // HAL_Delay(50); // used for debugging
    return 0;
  }
  return 1;
}
size_t pcap02_measure(uint8_t *data, SysTime_t ts) {
  // read sensor
  pcap02_result_t result;
  if (pcap02_measure_capacitance(&result) != 0) {
    return -1;
  }

  const UserConfiguration *cfg = UserConfigGet();

  // encode measurement
  size_t data_len = EncodePCAP02Measurement(
      ts.Seconds, cfg->logger_id, cfg->cell_id,
      PCAP02_REFERENCE_CAPACITOR_PF * fixed_to_float(&result), data);

  return data_len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  // Note: It takes about 1us after INTN

  if (GPIO_Pin == PCAP02_INTN_Pin) {
    INTN_State = (HAL_GPIO_ReadPin(PCAP02_INTN_GPIO_Port, PCAP02_INTN_Pin) ==
                  GPIO_PIN_SET); /* low active */
    if (INTN_State == GPIO_PIN_RESET) {
      INTN_Counter += 1;
    }
  }
}
