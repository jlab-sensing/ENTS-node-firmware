#ifndef PCAP02_H
#define PCAP02_H

#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "pcap02_standard.h"
#include "stm32_systime.h"

// Interrupt for PCAP02 INTN pin, which signals result ready
#define PCAP02_INTN_Pin GPIO_PIN_10
#define PCAP02_INTN_GPIO_Port GPIOA
#define PCAP02_INTN_EXTI_IRQn EXTI15_10_IRQn

// The address below is already left-shifted. The 8th bit (i.e. LSB) is the read
// (0) / write (1) bit.
#define PCAP02_I2C_ADDRESS 0x50

// Opcodes
#define PCAP02_OPCODE_OTP_WRITE 0xA0
#define PCAP02_OPCODE_OTP_READ 0x20
#define PCAP02_OPCODE_SRAM_WRITE 0x90
#define PCAP02_OPCODE_SRAM_READ 0x10
#define PCAP02_OPCODE_CONFIG_WRITE 0xC0
#define PCAP02_OPCODE_RESULT_READ 0x40
#define PCAP02_OPCODE_EEPROM_BLOCK_WRITE 0xE1
#define PCAP02_OPCODE_EEPROM_ERASE 0xE2
#define PCAP02_OPCODE_EEPROM_BLOCK_ERASE 0xE3
#define PCAP02_OPCODE_EEPROM_WRITE 0xE0
#define PCAP02_OPCODE_EEPROM_READ 0x60
#define PCAP02_OPCODE_POR 0x88
#define PCAP02_OPCODE_INITIALIZE 0x8A
#define PCAP02_OPCODE_CDC_START_CONVERSION 0x8C
#define PCAP02_OPCODE_RDC_START_CONVERSION 0x8E
#define PCAP02_OPCODE_OTP_WRITE_TERMINATE 0x84

// Address lengths
#define PCAP02_OTP_ADDRESS_LENGTH_BITS 13
#define PCAP02_SRAM_ADDRESS_LENGTH_BITS 12
#define PCAP02_EEPROM_ADDRESS_LENGTH_BITS 7
#define PCAP02_EEPROM_BLOCK_WRITE_ADDRESS_LENGTH_BITS 8
#define PCAP02_REGISTER_ADDRESS_LENGTH_BITS 7

// Config Registers
#define PCAP02_CONFIG_REGISTERS_LENGTH 78
#define PCAP02_CONFIG_REGISTERS_RUNBIT_REG 77

// Read Registers
#define PCAP02_READ_REGISTERS_LENGTH_BYTES 45
#define PCAP02_READ_REGISTERS_REG_LENGTH_BYTES 3
// Read Register offsets
#define PCAP02_READ_REGISTERS_RES0_OFFSET 0
#define PCAP02_READ_REGISTERS_RES1_OFFSET 3
#define PCAP02_READ_REGISTERS_RES2_OFFSET 6
#define PCAP02_READ_REGISTERS_RES3_OFFSET 9
#define PCAP02_READ_REGISTERS_RES4_OFFSET 12
#define PCAP02_READ_REGISTERS_RES5_OFFSET 15
#define PCAP02_READ_REGISTERS_RES6_OFFSET 18
#define PCAP02_READ_REGISTERS_RES7_OFFSET 21
#define PCAP02_READ_REGISTERS_STATUS_OFFSET 24
#define PCAP02_READ_REGISTERS_RES0_EXTEND_OFFSET 27
#define PCAP02_READ_REGISTERS_RES1_EXTEND_OFFSET 30
#define PCAP02_READ_REGISTERS_RES8_OFFSET 33
#define PCAP02_READ_REGISTERS_RES9_OFFSET 36
#define PCAP02_READ_REGISTERS_RES10_OFFSET 39
#define PCAP02_READ_REGISTERS_RES11_OFFSET 42

// Types

typedef union {
  uint8_t byte[3];
  struct {
    struct {
      uint8_t RunBit : 1;  // bit 0
      uint8_t CDC_active : 1;
      uint8_t RDC_ready : 1;
      uint8_t EEPROM_busy : 1;
      uint8_t AutoBoot_busy : 1;
      uint8_t POR_Flag_SRAM : 1;
      uint8_t POR_Flag_Config : 1;
      uint8_t POR_Flag_Wdog : 1;  // bit 7
    } STATUS_0;
    struct {
      uint8_t Comb_Err : 1;
      uint8_t Err_Ovfl : 1;
      uint8_t Mup_Err : 1;
      uint8_t RDC_Err : 1;
      uint8_t : 4;  // bit 4 - 7 n.c.
    } STATUS_1;
    struct {
      uint8_t C_PortError0 : 1;  // PC0
      uint8_t C_PortError1 : 1;  // PC1
      uint8_t C_PortError2 : 1;  // PC2
      uint8_t C_PortError3 : 1;  // PC3
      uint8_t C_PortError4 : 1;  // PC4
      uint8_t C_PortError5 : 1;  // PC5
      uint8_t C_PortError6 : 1;  // PC6
      uint8_t C_PortError7 : 1;  // PC7
    } STATUS_2;
  };
} pcap02_read_register_status_t;

// Global Variables

// High level functions
void pcap02_init(void);
void pcap02_gpio_init(void);
size_t pcap02_measure_capacitance(pcap02_result_t *result);
size_t pcap02_measure(uint8_t *data, SysTime_t ts);

uint16_t pcap02_sram_write_firmware(uint8_t *firmware, uint16_t offset_bytes,
                                    uint16_t length_bytes);
void pcap02_print_status_register(pcap02_read_register_status_t status);

// Low level I2C functions
void I2C_Sweep_DevAddr(uint8_t from_addr, uint8_t to_addr, uint8_t *addr_array);
HAL_StatusTypeDef I2C_Write_Opcode(uint8_t slave, uint8_t one_byte);
HAL_StatusTypeDef I2C_Memory_Access(uint8_t slave, uint8_t opcode,
                                    uint16_t address, uint8_t *byte,
                                    uint16_t size);
HAL_StatusTypeDef I2C_Config_Access(uint8_t slave, uint8_t opcode,
                                    uint8_t address, uint8_t *byte,
                                    uint8_t size);

HAL_StatusTypeDef I2C_Write_Dword(uint8_t slave, uint8_t opcode,
                                  uint8_t address, uint32_t dword);
HAL_StatusTypeDef I2C_Write_Byte(uint8_t slave, uint8_t opcode, uint8_t address,
                                 uint8_t byte);

uint32_t I2C_Read_Dword(uint8_t slave, uint8_t rd_opcode, uint8_t address);
uint8_t I2C_Read_Byte(uint8_t slave, uint8_t rd_opcode, uint8_t address);
uint32_t I2C_Read_Result(uint8_t slave, uint8_t rd_opcode, uint8_t address);

#endif