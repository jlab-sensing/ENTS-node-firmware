/**
 * @example example_pcap02.c
 *
 * Prints the C1 / C0 (single floating) capacitance ratio from the PCAP02
 * capacitance to digital sensor. C0 is the reference capacitor connected across
 * PC0 and PC1 of the plugin module. C1 is the capacitance measured across the
 * PC2 and PC3 ports.
 *
 * @author Jack Lin
 * @date 2025-08-18
 */

// Includes
#include <stdio.h>

#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "pcap02.h"
#include "pcap02_standard.h"
#include "sys_app.h"
#include "usart.h"

static HAL_StatusTypeDef ret = HAL_OK;
static volatile uint16_t dev_addr = PCAP02_I2C_ADDRESS;

static volatile pcap02_result_t RES1;

volatile uint32_t My_INTN_Counter = 0;
volatile uint8_t My_INTN_State = 1;

static uint8_t runbit = 0;

pcap02_read_register_status_t status_register;

/* Private function prototypes -----------------------------------------------*/
float fixed_to_float(pcap02_result_t RESx);
uint32_t test_sram_write_byte(uint8_t txData, uint16_t location);
uint32_t test_sram_write_memory_access(uint8_t txData[], uint8_t rxData[],
                                       uint16_t location, uint32_t length);
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();

  pcap02_init();

  /*
  APP_LOG(TS_OFF, VLEVEL_M,"Sweep I2C addresses.\r\n");
  uint8_t slave[255] = {0};
  I2C_Sweep_DevAddr(0, 127, slave);
  for (uint8_t i = 0; i < sizeof(slave); i++) {
    if (slave[i] == 0) continue;
    APP_LOG(TS_OFF, VLEVEL_M,"%d 0x%02X\r\n", i, slave[i]);
  }
  */

  runbit = 0x00;
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_CONFIG_WRITE, PCAP02_CONFIG_REGISTERS_RUNBIT_REG,
      &runbit,
      1);  // Disable reg 77 runbit as recommended before any memory access.

  APP_LOG(TS_OFF, VLEVEL_M,
          "OPCODE POR: Writing opcode 0x%02X to device 0x%02X\r\n",
          PCAP02_OPCODE_POR, dev_addr);
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_POR);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(500);

  if (test_sram_write_byte(0xA5, 0x047)) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to write byte to pcap02 sram.\r\n");
    Error_Handler();
  }

  uint8_t txData[100];
  for (uint16_t i = 0; i < sizeof(txData); i++) {
    txData[i] = i;
    APP_LOG(TS_OFF, VLEVEL_M, "%d ", txData[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_M, "\r\n");
  uint8_t rxData[sizeof(txData)];
  if (test_sram_write_memory_access(txData, rxData, 0, sizeof(txData))) {
    APP_LOG(TS_OFF, VLEVEL_M,
            "Failed to write memory access to pcap02 sram.\r\n");
    Error_Handler();
  }

  APP_LOG(TS_OFF, VLEVEL_M, "\r\nTests concluded.\r\n\r\n");

  // POR + INIT
  APP_LOG(TS_OFF, VLEVEL_M, "Power On Reset\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_POR);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(500);
  APP_LOG(TS_OFF, VLEVEL_M, "Initialize\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_INITIALIZE);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  HAL_Delay(10);

  // Write configuration registers
  APP_LOG(TS_OFF, VLEVEL_M, "Setting runbit to 0.\r\n");
  runbit = 0x00;
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_CONFIG_WRITE, PCAP02_CONFIG_REGISTERS_RUNBIT_REG,
      &runbit,
      1);  // Disable reg 77 runbit as recommended before any memory access.
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  APP_LOG(TS_OFF, VLEVEL_M,
          "Writing to all (non-runbit) config registers.\r\n");
  ret = I2C_Config_Access(dev_addr, PCAP02_OPCODE_CONFIG_WRITE, 0,
                          pcap02_standard_config_registers,
                          PCAP02_CONFIG_REGISTERS_LENGTH -
                              1);  // Do not write reg 77 RUNBIT to 1 yet.
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  // Write firmware with additional write verification of e.g. 100 bytes

  // ret = I2C_Memory_Access(dev_addr, PCAP02_OPCODE_SRAM_WRITE, 0x00,
  // standard_fw, 548); ret = I2C_Memory_Access(dev_addr,
  // PCAP02_OPCODE_SRAM_READ, 0x00, My_buf, 100);

  APP_LOG(TS_OFF, VLEVEL_M, "Writing standard firmware  to SRAM.\r\n");
  pcap02_sram_write_firmware(pcap02_standard_firmware,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_OFFSET +
                                 PCAP02_STANDARD_FIRMWARE_MAIN_OFFSET,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_LENGTH);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_M, "Writing standard firmware version to SRAM.\r\n");
  pcap02_sram_write_firmware(pcap02_standard_firmware_version,
                             PCAP02_STANDARD_FIRMWARE_PROGRAM_OFFSET +
                                 PCAP02_STANDARD_FIRMWARE_VERSION_OFFSET,
                             PCAP02_STANDARD_FIRMWARE_VERSION_LENGTH);
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  // Start CDC measurement
  APP_LOG(TS_OFF, VLEVEL_M,
          "Reading status register. (Before runbit=1 and final "
          "initialization)\r\n");
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_RESULT_READ, PCAP02_READ_REGISTERS_STATUS_OFFSET,
      status_register.byte,
      PCAP02_READ_REGISTERS_REG_LENGTH_BYTES);  // Read status register
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  pcap02_print_status_register(status_register);

  APP_LOG(TS_OFF, VLEVEL_M, "Setting runbit to 1.\r\n");
  runbit = 0x01;
  ret = I2C_Config_Access(dev_addr, PCAP02_OPCODE_CONFIG_WRITE,
                          PCAP02_CONFIG_REGISTERS_RUNBIT_REG, &runbit,
                          1);  // Enable reg 77 runbit.
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_M, "Initialize.\r\n");
  ret = I2C_Write_Opcode(dev_addr, PCAP02_OPCODE_INITIALIZE);  // Partial reset
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(TS_OFF, VLEVEL_M, "CDC Start Conversion.\r\n");
  ret = I2C_Write_Opcode(
      dev_addr, PCAP02_OPCODE_CDC_START_CONVERSION);  // Start measurement
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);

  APP_LOG(
      TS_OFF, VLEVEL_M,
      "Reading status register. (After runbit=1 and final initialization)\r\n");
  ret = I2C_Config_Access(
      dev_addr, PCAP02_OPCODE_RESULT_READ, PCAP02_READ_REGISTERS_STATUS_OFFSET,
      status_register.byte,
      PCAP02_READ_REGISTERS_REG_LENGTH_BYTES);  // Read status register
  APP_LOG(TS_OFF, VLEVEL_M, "\tret (HAL) %d\r\n", ret);
  pcap02_print_status_register(status_register);

  // while (1)
  // {
  //     HAL_Delay(1);
  // }

  // 8. ‘h40 03 00 00 00; Read Res1, addresses 3, 4, 5. Res1 is expected to be
  //                      in the range of 2,000,000 or ’h2000XX if the two
  //                      capacitors are of same size. Res1 has the format of a
  //                      fixed point number with 3 integer digits and 21
  //                      fractional digits. So, dividing the 2,000,000 by 2^21
  //                      gives a factor of about 1 for the ratio C1/C0.
  APP_LOG(TS_OFF, VLEVEL_M, "sizeof(RES1) = %d\r\n", sizeof(RES1));
  while (1) {
    // Read result register after INTN = 0
    // Note: Approximately 82 ms or 83 ms between prints.
    if (My_INTN_State == 0) {
      APP_LOG(
          TS_OFF, VLEVEL_M,
          "[reading %lu @ tick %lu] Read Result on RES1 (ratio of C1 / C0)\r\n",
          My_INTN_Counter, HAL_GetTick());
      // MyRawRES0 = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ, 0x00);
      // MyRawRES1 = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ, 0x04);
      RES1.word = I2C_Read_Result(dev_addr, PCAP02_OPCODE_RESULT_READ,
                                  PCAP02_READ_REGISTERS_RES1_OFFSET);
      APP_LOG(TS_OFF, VLEVEL_M,
              "\tbytes: 0x%02X%02X%02X\r\n\t24-bit: 0x%06X\r\n\tfixed: "
              "%01d\r\n\tfractional (raw): %d\r\n\tfloat RATIO: %f\r\n",
              RES1.byte[2], RES1.byte[1], RES1.byte[0], RES1.word, RES1.fixed,
              RES1.fractional, fixed_to_float(RES1));

      // Post Processing
      // MyRatioRES0 = (float)MyRawRES0 / 134217728; // = 2^27
      // MyRatioRES1 = (float)MyRawRES1 / 134217728; // = 2^27

      // APP_LOG(TS_OFF, VLEVEL_M,"\r\nRES0: raw %ld ratio %f\r\nRES1: raw %ld
      // ratio %f\r\n", MyRawRES0, MyRatioRES0, MyRawRES1, MyRatioRES1);
      // HAL_Delay(50); // used for debugging
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  // Note: It takes about 1us after INTN

  if (GPIO_Pin == PCAP02_INTN_Pin) {
    My_INTN_State = (HAL_GPIO_ReadPin(PCAP02_INTN_GPIO_Port, PCAP02_INTN_Pin) ==
                     GPIO_PIN_SET); /* low active */
    if (My_INTN_State == 0) {
      My_INTN_Counter += 1;
    }
  }
}

float fixed_to_float(pcap02_result_t RESx) {
  return ((float)RESx.fixed) +
         ((float)(RESx.fractional) /
          (1 << PCAP02_STANDARD_FIRMWARE_RESULT_FRACTIONAL_BITS));
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