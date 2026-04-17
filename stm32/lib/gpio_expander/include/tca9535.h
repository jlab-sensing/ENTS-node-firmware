/**
 * @file tca9535.h
 * @author Jack Lin (jlin143@ucsc.edu)
 * @brief
 * @version 1.0
 * @date 2026-04-10
 *
 * @note Adapted from TI SLVC564 I/O Expander Software and Firmware Package. Interrupt callback located in lora_app.c.
 *
 * TODO:
 * - n/a
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef LIB_GPIO_EXPANDER_SRC_TCA9535_H_
#define LIB_GPIO_EXPANDER_SRC_TCA9535_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "main.h"

/************************** I2C Address ***************************************/
#define TCA9535_ADDRESS (0x20 << 1)  // I2C 7-bit Address [0100(A2)(A1)(A0)] + (R/~W)

/************************** I2C Registers *************************************/
#define TCA9535_INPUT_REG0 0x00  // Input status register
#define TCA9535_INPUT_REG1 0x01  // Input status register
#define TCA9535_OUTPUT_REG0 0x02  // Output register to change state of output BIT set to 1, output set HIGH
#define TCA9535_OUTPUT_REG1 0x03  // Output register to change state of output BIT set to 1, output set HIGH
#define TCA9535_POLARITY_REG0 0x04  // Polarity inversion register. BIT '1' inverts input polarity of register 0x00 (input registers only)
#define TCA9535_POLARITY_REG1 0x05  // Polarity inversion register. BIT '1' inverts input polarity of register 0x01 (input registers only)
#define TCA9535_CONFIG_REG0 0x06  // Configuration register. BIT = '1' sets port to input BIT = '0' sets port to output
#define TCA9535_CONFIG_REG1 0x07  // Configuration register. BIT = '1' sets port to input BIT = '0' sets port to output

/************************** Register Options *************************************/
#define TCA9535_CONFIG_OUTPUT 0
#define TCA9535_CONFIG_INPUT 1
#define TCA9535_POLARITY_ACTIVE_HIGH 0
#define TCA9535_POLARITY_ACTIVE_LOW 1

/************************** Pin Map *************************************/
// This pin mapping section describes the pin connection for the onboard TCA9535 on hardware v3.1.0.

// P0: [MSB] P07 ... P00 [LSB]
// P1: [MSB] P17 ... P10 [LSB]
//            ^port    ^bit

// HW v3.1.0

// P0
// 0 ESP32 WAKEUP (IO3)
// 1 ~CHG
// 2 ~PG
// 3 USART1_SHUTDOWN
// 4 I2C1_PORT1_SHUTDOWN
// 5 I2C1_PORT2_SHUTDOWN
// 6 SPI2_SHUTDOWN
// 7 SOLAR_DETECT

// P1
// 0 27
// 1 29
// 2 31
// 3 33
// 4 35
// 5 37
// 6 28
// 7 36

#define TCA9535_WAKEUP_PORT 0
#define TCA9535_WAKEUP_PIN 0
#define TCA9535_WAKEUP_MASK ((1 << TCA9535_WAKEUP_PIN) << (TCA9535_WAKEUP_PORT * 8))
#define TCA9535_nCHG_PORT 0
#define TCA9535_nCHG_PIN 1
#define TCA9535_nCHG_MASK ((1 << TCA9535_nCHG_PIN) << (TCA9535_nCHG_PORT * 8))
#define TCA9535_nPG_PORT 0
#define TCA9535_nPG_PIN 2
#define TCA9535_nPG_MASK ((1 << TCA9535_nPG_PIN) << (TCA9535_nPG_PORT * 8))
#define TCA9535_USART1_SHUTDOWN_PORT 0
#define TCA9535_USART1_SHUTDOWN_PIN 3
#define TCA9535_USART1_SHUTDOWN_MASK ((1 << TCA9535_USART1_SHUTDOWN_PIN) << (TCA9535_USART1_SHUTDOWN_PORT * 8))
#define TCA9535_I2C1_PORT1_SHUTDOWN_PORT 0
#define TCA9535_I2C1_PORT1_SHUTDOWN_PIN 4
#define TCA9535_I2C1_PORT1_SHUTDOWN_MASK ((1 << TCA9535_I2C1_PORT1_SHUTDOWN_PIN) << (TCA9535_I2C1_PORT1_SHUTDOWN_PORT * 8))
#define TCA9535_I2C1_PORT2_SHUTDOWN_PORT 0
#define TCA9535_I2C1_PORT2_SHUTDOWN_PIN 5
#define TCA9535_I2C1_PORT2_SHUTDOWN_MASK ((1 << TCA9535_I2C1_PORT2_SHUTDOWN_PIN) << (TCA9535_I2C1_PORT2_SHUTDOWN_PORT * 8))
#define TCA9535_SPI2_SHUTDOWN_PORT 0
#define TCA9535_SPI2_SHUTDOWN_PIN 6
#define TCA9535_SPI2_SHUTDOWN_MASK ((1 << TCA9535_SPI2_SHUTDOWN_PIN) << (TCA9535_SPI2_SHUTDOWN_PORT * 8))
#define TCA9535_SOLAR_DETECT_PORT 0
#define TCA9535_SOLAR_DETECT_PIN 7
#define TCA9535_SOLAR_DETECT_MASK ((1 << TCA9535_SOLAR_DETECT_PIN) << (TCA9535_SOLAR_DETECT_PORT * 8))

// Unused
#define TCA9535_P10_PORT 1
#define TCA9535_P10_PIN 0
#define TCA9535_P10_MASK ((1 << TCA9535_P10_PIN) << (TCA9535_P10_PORT * 8))
#define TCA9535_P11_PORT 1
#define TCA9535_P11_PIN 1
#define TCA9535_P11_MASK ((1 << TCA9535_P11_PIN) << (TCA9535_P11_PORT * 8))
#define TCA9535_P12_PORT 1
#define TCA9535_P12_PIN 2
#define TCA9535_P12_MASK ((1 << TCA9535_P12_PIN) << (TCA9535_P12_PORT * 8))
#define TCA9535_P13_PORT 1
#define TCA9535_P13_PIN 3
#define TCA9535_P13_MASK ((1 << TCA9535_P13_PIN) << (TCA9535_P13_PORT * 8))
#define TCA9535_P14_PORT 1
#define TCA9535_P14_PIN 4
#define TCA9535_P14_MASK ((1 << TCA9535_P14_PIN) << (TCA9535_P14_PORT * 8))
#define TCA9535_P15_PORT 1
#define TCA9535_P15_PIN 5
#define TCA9535_P15_MASK ((1 << TCA9535_P15_PIN) << (TCA9535_P15_PORT * 8))
#define TCA9535_P16_PORT 1
#define TCA9535_P16_PIN 6
#define TCA9535_P16_MASK ((1 << TCA9535_P16_PIN) << (TCA9535_P16_PORT * 8))
#define TCA9535_P17_PORT 1
#define TCA9535_P17_PIN 7
#define TCA9535_P17_MASK ((1 << TCA9535_P17_PIN) << (TCA9535_P17_PORT * 8))

/************************** Structs and Unions *************************************/
struct TCA9535_sBit {
  uint8_t B0 : 1;
  uint8_t B1 : 1;
  uint8_t B2 : 1;
  uint8_t B3 : 1;
  uint8_t B4 : 1;
  uint8_t B5 : 1;
  uint8_t B6 : 1;
  uint8_t B7 : 1;
};

union TCA9535_uInputPort {
  uint8_t all;
  struct TCA9535_sBit bit;
};

struct TCA9535_sInput {
  union TCA9535_uInputPort P0;
  union TCA9535_uInputPort P1;
};

union TCA9535_uInput {
  uint16_t all;
  struct TCA9535_sInput Port;
};

union TCA9535_uBitAll {
  uint8_t all;
  struct TCA9535_sBit bit;
};
/*
union TCA9535_uOutputP1{
        uint8_t 		all;
        struct TCA9535_sBit	bit;
};*/

struct TCA9535_sOutput {
  union TCA9535_uBitAll P0;
  union TCA9535_uBitAll P1;
};

union TCA9535_uOutput {
  uint16_t all;
  struct TCA9535_sOutput Port;
};
/*
union TCA9535_uPolarityInversionP0{
        uint8_t all;
        struct TCA9535_sBit bit;
};

union  TCA9535_uPolarityInversionP1{
        uint8_t	all;
        struct TCA9535_sBit bit;
};
*/
struct TCA9535_sPolarityInversion {
  union TCA9535_uBitAll P0;
  union TCA9535_uBitAll P1;
};

union TCA9535_uPolarityInversion {
  uint16_t all;
  struct TCA9535_sPolarityInversion Port;
};
/*
union TCA9535_uConfigP0{
        uint8_t all;
        struct TCA9535_sBit bit;
};

union  TCA9535_uConfigP1{
        uint8_t	all;
        struct TCA9535_sBit bit;
};*/

struct TCA9535_sConfig {
  union TCA9535_uBitAll P0;
  union TCA9535_uBitAll P1;
};

union TCA9535_uConfig {
  uint16_t all;
  struct TCA9535_sConfig Port;
};

// struct sTCA9535Regs{
// 	union TCA9535_uInput 				Input;
// 	union TCA9535_uOutput 				Output;
// 	union TCA9535_uPolarityInversion 	PolarityInversion;
// 	union TCA9535_uConfig				Config;
// };

typedef struct {
  union TCA9535_uInput Input;
  union TCA9535_uOutput Output;
  union TCA9535_uPolarityInversion PolarityInversion;
  union TCA9535_uConfig Config;
} TCA9535Regs;

/************************** Global Variables *************************************/
extern TCA9535Regs TCA9535_Reg_map;

/************************** Public Function Prototypes *************************************/


// High level functions
bool TCA9535Init(bool interruptEnable);

void Tca9535WritePin(uint8_t IO_Port, uint8_t IO_Pin, GPIO_PinState PinState);
void Tca9535TogglePin(uint8_t IO_Port, uint8_t IO_Pin);
GPIO_PinState Tca9535ReadPin(uint8_t IO_Port, uint8_t IO_Pin);
void Tca9535SetDirection(uint8_t IO_Port, uint8_t IO_Pin, uint8_t Direction);

// Low level functions
void TCA9535InitStructDefault(TCA9535Regs* Regs);
void Tca9535InterruptInit();
void Tca9535InterruptDeinit();

HAL_StatusTypeDef TCA9535WriteOutput(TCA9535Regs* Regs);
HAL_StatusTypeDef TCA9535WritePolarity(TCA9535Regs* Regs);
HAL_StatusTypeDef TCA9535WriteConfig(TCA9535Regs* Regs);

HAL_StatusTypeDef TCA9535WriteAll(TCA9535Regs* Regs);

HAL_StatusTypeDef TCA9535ReadInput(TCA9535Regs* Regs);
HAL_StatusTypeDef TCA9535ReadOutput(TCA9535Regs* Regs);
HAL_StatusTypeDef TCA9535ReadPolarity(TCA9535Regs* Regs);
HAL_StatusTypeDef TCA9535ReadConfig(TCA9535Regs* Regs);

HAL_StatusTypeDef TCA9535ReadAll(TCA9535Regs* Regs);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_GPIO_EXPANDER_SRC_TCA9535_H_
