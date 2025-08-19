#include "pcap02.h"

#include "board.h"
#include "i2c.h"

// Private Globals
static HAL_StatusTypeDef ret = HAL_OK;
static volatile uint16_t dev_addr = PCAP02_I2C_ADDRESS;

// Function Definitions

void pcap02_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : INTN_Pin */
  GPIO_InitStruct.Pin = PCAP02_INTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PCAP02_INTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(PCAP02_INTN_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PCAP02_INTN_EXTI_IRQn);
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

  printf("\r\npcap02_sram_write_firmware START\r\n");

  printf("\tFirmware to write (hex):");
  printf("\r\n\t\t         +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F");
  if (offset_bytes % 16 != 0) {
    printf("\r\n\t\t[0x%03X]: ", offset_bytes - (offset_bytes % 16));
    for (uint16_t i = 0; i < (offset_bytes % 16); i++) {
      printf("__ ");
    }
  }
  for (uint16_t i = 0; i < length_bytes; i++) {
    if (((i + (offset_bytes % 16)) % 16) == 0) {
      printf("\r\n\t\t[0x%03X]: ", ((i + (offset_bytes % 16)) / 16) << 4);
    }
    printf("%02X ", firmware[i]);
  }
  printf("\r\n");

  printf("\tWriting %d bytes to SRAM.\r\n", length_bytes);
  ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_WRITE, offset_bytes,
                          firmware, length_bytes);
  printf("\t\tret (HAL) %d\r\n", ret);

  printf("\tReading %d bytes from SRAM.\r\n", length_bytes);
  ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_READ, offset_bytes,
                          rxDataLong, length_bytes);
  printf("\t\tret (HAL) %d\r\n", ret);

  printf("\t\t(Received firmware omitted.)\r\n");
  // for (uint16_t i = 0; i < sizeof(rxDataLong); i++)
  // {
  //   printf("%d ", rxDataLong[i]);
  // }

  printf("\tVerify transmitted and received firmware.\r\n");
  uint16_t error_count = 0;
  for (uint16_t i = 0; i < length_bytes; i++) {
    if (firmware[i] != rxDataLong[i]) {
      error_count += 1;
    }
  }
  printf("\t\tErrors: %d --> %s\r\n", error_count,
         (error_count) ? ("FAIL") : ("PASS"));

  printf("\r\npcap02_sram_write_firmware END\r\n");

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
  printf("0x%02X%02X%02X\r\n", status.byte[0], status.byte[1], status.byte[2]);

  printf(
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
  printf(
      "STATUS_1:\r\n"
      "\t.0 Comb_Err: %d\r\n"
      "\t.2 Err_Ovfl: %d\r\n"
      "\t.3 Mup_Err: %d\r\n"
      "\t.4 RDC_Err: %d\r\n",
      status.STATUS_1.Comb_Err, status.STATUS_1.Err_Ovfl,
      status.STATUS_1.Mup_Err, status.STATUS_1.RDC_Err);
  printf(
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
 * @param  slave (7 Bit) decive address
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
 * @param  slave (7 Bit) decive address
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
 * @param  slave (7 Bit) decive address
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