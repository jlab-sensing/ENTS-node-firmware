#include "tca9535.h"

#include "i2c.h"
// #include "stm32wlxx_ll_i2c.h"
#include "sys_app.h"  // APP_LOG

TCA9535Regs TCA9535_Reg_map;

static const uint32_t timeout = HAL_MAX_DELAY;

/**
 * @brief Initializes the TCA9535 with the ENTS HW v3.1.0 default configuration.
 * 
 * @param interruptEnable bool to optionally enable the external interrupt for
 * TCA9535 pin change.
 * 
 * @note The default configuration is different from the TCA9535 power-on device
 * default settings.
 * 
 * @return void
 */
bool TCA9535Init(bool interruptEnable) {
  TCA9535InitStructDefault((TCA9535Regs*)&TCA9535_Reg_map);

  if (TCA9535WriteAll((TCA9535Regs*)&TCA9535_Reg_map) != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error initializing TCA9535 IO Expander.\r\n");
    return false;
  }

  if (interruptEnable) {
    TCA9535InterruptInit();
  }

  return true;
}

/**
 * @brief Initializes pin PB13 as an external interrupt to receive the active
 * low interrupt signal from the TCA9535.
 *
 * @note (Hardware v3.1.0) Requires solder jumper JP9 to be closed to connect
 * ~INT with PB13.
 *
 * @note INT is driven low (valid after max 4 us) upon any input pin change.
 * @note INT signal is reset after input value returns to original setting or
 * data is read from the port that generated the interrupt. INT reset delay time
 * max 4 us.
 *
 * @return void
 */
void TCA9535InterruptInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : INTN_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // APP_LOG(TS_OFF, VLEVEL_M, "TCA9535 IO Expander interrupt enabled. [HW
  // v3.1.0] JP9 must be soldered.\r\n");
}

/**
 * @brief Deinitializes pin PB13 as an external interrupt to receive the active
 * low interrupt signal from the TCA9535. Interrupt is disabled and PB13 is
 * restored as an analog input.
 *
 * @return void
 */
void TCA9535InterruptDeinit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : INTN_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

// ****************************************************************************
//! @fn          void TCA9535InitStructDefault(TCA9535Regs* Regs)
//! @brief
//!				 Initializes the TCA9535Regs registers structure
//! which is
//!        a local mirror of the TCA9535 control registers.
//!        An additional TCA9535WriteAll(Regs) function call
//! 			 is needed in order to get the TCA9535 registers updated
//! @note  Defaults:
//!        P00 Wakeup:                Output, active high, output 0
//!        P01 nCHG:                  Input,  active high, output 0
//!        P02 nPG:                   Input,  active high, output 0
//!        P03 USART1 Shutdown:       Output, active high, output 0
//!        P04 I2C1 Port 1 Shutdown:  Output, active high, output 0
//!        P05 I2C1 Port 2 Shutdown:  Output, active high, output 0
//!        P06 SPI2 Shutdown:         Output, active high, output 0
//!        P07 Solar Detect:          Input,  active high, output 0
//!        P10-17 unused:             Input,  active high, output 0
// ****************************************************************************
void TCA9535InitStructDefault(TCA9535Regs* Regs) {
  Regs->Output.all = 0x0000;
  Regs->PolarityInversion.all = 0x0000;
  Regs->Config.all =
      0xFFFF & ~(TCA9535_WAKEUP_MASK | TCA9535_USART1_SHUTDOWN_MASK |
                 TCA9535_I2C1_PORT1_SHUTDOWN_MASK |
                 TCA9535_I2C1_PORT2_SHUTDOWN_MASK | TCA9535_SPI2_SHUTDOWN_MASK);
}

// ****************************************************************************
//! @fn          HAL_StatusTypeDef TCA9535WriteAll(TCA9535Regs* Regs)
//! @brief Initializes the the config (IO direction), output, and polarity
//! registers
// ****************************************************************************
HAL_StatusTypeDef TCA9535WriteAll(TCA9535Regs* Regs) {
  HAL_StatusTypeDef hal_status = HAL_OK;

  // Registers can only be sequentially written in the same transmission by
  // pairs (i.e. CONFIG0, then CONFIG1), not between different register types
  // (i.e. OUTPUT1 then POLARITY0).

  hal_status = TCA9535WriteOutput(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = TCA9535WritePolarity(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = TCA9535WriteConfig(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  return hal_status;
}

HAL_StatusTypeDef TCA9535ReadAll(TCA9535Regs* Regs) {
  HAL_StatusTypeDef hal_status = HAL_OK;

  hal_status = TCA9535ReadInput(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = TCA9535ReadConfig(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = TCA9535ReadOutput(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  hal_status = TCA9535ReadPolarity(Regs);
  if (hal_status != HAL_OK) {
    return hal_status;
  }

  return hal_status;
}

HAL_StatusTypeDef TCA9535ReadInput(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Read(&hi2c1, TCA9535_ADDRESS, TCA9535_INPUT_REG0,
                          TCA9535_MEM_ADDRESS_SIZE, (uint8_t*)&Regs->Input,
                          sizeof(Regs->Input), timeout);
}

HAL_StatusTypeDef TCA9535WriteOutput(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Write(&hi2c1, TCA9535_ADDRESS, TCA9535_OUTPUT_REG0,
                           TCA9535_MEM_ADDRESS_SIZE, (uint8_t*)&Regs->Output,
                           sizeof(Regs->Output), timeout);
}

HAL_StatusTypeDef TCA9535ReadOutput(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Read(&hi2c1, TCA9535_ADDRESS, TCA9535_OUTPUT_REG0,
                          TCA9535_MEM_ADDRESS_SIZE, (uint8_t*)&Regs->Output,
                          sizeof(Regs->Output), timeout);
}

HAL_StatusTypeDef TCA9535WritePolarity(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Write(&hi2c1, TCA9535_ADDRESS, TCA9535_POLARITY_REG0,
                           TCA9535_MEM_ADDRESS_SIZE,
                           (uint8_t*)&Regs->PolarityInversion,
                           sizeof(Regs->PolarityInversion), timeout);
}

HAL_StatusTypeDef TCA9535ReadPolarity(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Read(&hi2c1, TCA9535_ADDRESS, TCA9535_POLARITY_REG0,
                          TCA9535_MEM_ADDRESS_SIZE,
                          (uint8_t*)&Regs->PolarityInversion,
                          sizeof(Regs->PolarityInversion), timeout);
}

HAL_StatusTypeDef TCA9535WriteConfig(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Write(&hi2c1, TCA9535_ADDRESS, TCA9535_CONFIG_REG0,
                           TCA9535_MEM_ADDRESS_SIZE, (uint8_t*)&Regs->Config,
                           sizeof(Regs->Config), timeout);
}

HAL_StatusTypeDef TCA9535ReadConfig(TCA9535Regs* Regs) {
  return HAL_I2C_Mem_Read(&hi2c1, TCA9535_ADDRESS, TCA9535_CONFIG_REG0,
                          TCA9535_MEM_ADDRESS_SIZE, (uint8_t*)&Regs->Config,
                          sizeof(Regs->Config), timeout);
}

/**
 * @brief Set or reset an output register bit on the onboard TCA9535.
 *
 * @note Pin direction must be set independently.
 * 
 * @param IO_Port  Port number
 * @param IO_Pin   Pin number
 * @param PinState Pin state to apply
 * @return void
 */
void TCA9535WritePin(uint8_t IO_Port, uint8_t IO_Pin, GPIO_PinState PinState) {
  TCA9535_Reg_map.Output.all =
      (TCA9535_Reg_map.Output.all & ~((1 << IO_Pin) << (IO_Port * 8))) |
      ((PinState << IO_Pin) << (IO_Port * 8));
  APP_LOG(TS_OFF, VLEVEL_M, "\tTCA9535_Reg_map.Output.all = 0x%04X\r\n", TCA9535_Reg_map.Output.all);

  APP_LOG(TS_OFF, VLEVEL_M, "\t%d\r\n", TCA9535WriteOutput(&TCA9535_Reg_map));
}

/**
 * @brief Toggle an output register bit on the onboard TCA9535.
 *
 * @note Pin direction must be set independently.
 * 
 * @param IO_Port Port number
 * @param IO_Pin  Pin number
 * @return void
 */
void TCA9535TogglePin(uint8_t IO_Port, uint8_t IO_Pin) {
  TCA9535_Reg_map.Output.all ^= (1 << IO_Pin) << (IO_Port * 8);

  TCA9535WriteOutput(&TCA9535_Reg_map);
}

/**
 * @brief Read an input register bit on the onboard TCA9535.
 *
 * @param IO_Port Port number
 * @param IO_Pin  Pin number
 * @return See GPIO_PinState
 */
GPIO_PinState TCA9535ReadPin(uint8_t IO_Port, uint8_t IO_Pin) {
  if (TCA9535ReadInput(&TCA9535_Reg_map) != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error reading TCA9535 input register.\r\n");
  }
  return ((TCA9535_Reg_map.Input.all >> IO_Pin) >> (IO_Port * 8)) & 1;
}

/**
 * @brief Set pin direction for the onboard TCA9535.
 * @param IO_Port Port number
 * @param IO_Pin  Pin number
 * @param Direction TCA9535_CONFIG_OUTPUT or TCA9535_CONFIG_INPUT
 */
void TCA9535SetDirection(uint8_t IO_Port, uint8_t IO_Pin, uint8_t Direction) {
  TCA9535_Reg_map.Config.all =
      (TCA9535_Reg_map.Config.all & ~((1 << IO_Pin) << (IO_Port * 8))) |
      ((Direction << IO_Pin) << (IO_Port * 8));
  APP_LOG(TS_OFF, VLEVEL_M, "\tTCA9535_Reg_map.Config.all = 0x%04X\r\n", TCA9535_Reg_map.Config.all);

  APP_LOG(TS_OFF, VLEVEL_M, "\t%d\r\n", TCA9535WriteConfig(&TCA9535_Reg_map));
  
}

/**
 * @brief Prints the values within the TCA9535Regs struct argument.
 */
void TCA9535PrintAll(TCA9535Regs* Regs){
  
  APP_LOG(TS_OFF, VLEVEL_M, "Config.all = 0x%04X\r\n", TCA9535_Reg_map.Config.all);
  // APP_LOG(TS_OFF, VLEVEL_M, "Config.bits = 0b%01d%01d%01d%01d_%01d%01d%01d%01d\r\n", 
  //   TCA9535_Reg_map.Config.Port.P1.bit.B7, 
  //   TCA9535_Reg_map.Config.Port.P1.bit.B6,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B5,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B4,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B3,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B2,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B1,
  //   TCA9535_Reg_map.Config.Port.P1.bit.B0
  // );
  APP_LOG(TS_OFF, VLEVEL_M, "Input.all = 0x%04X\r\n", TCA9535_Reg_map.Input.all);
  // APP_LOG(TS_OFF, VLEVEL_M, "Input.bits = 0b%01d%01d%01d%01d_%01d%01d%01d%01d\r\n", 
  //   TCA9535_Reg_map.Input.Port.P1.bit.B7, 
  //   TCA9535_Reg_map.Input.Port.P1.bit.B6,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B5,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B4,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B3,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B2,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B1,
  //   TCA9535_Reg_map.Input.Port.P1.bit.B0
  // );
  APP_LOG(TS_OFF, VLEVEL_M, "Output.all = 0x%04X\r\n", TCA9535_Reg_map.Output.all);
  // APP_LOG(TS_OFF, VLEVEL_M, "Output.bits = 0b%01d%01d%01d%01d_%01d%01d%01d%01d\r\n", 
  //   TCA9535_Reg_map.Output.Port.P1.bit.B7, 
  //   TCA9535_Reg_map.Output.Port.P1.bit.B6,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B5,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B4,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B3,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B2,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B1,
  //   TCA9535_Reg_map.Output.Port.P1.bit.B0
  // );
  APP_LOG(TS_OFF, VLEVEL_M, "PolarityInversion.all = 0x%04X\r\n", TCA9535_Reg_map.PolarityInversion.all);
  // APP_LOG(TS_OFF, VLEVEL_M, "PolarityInversion.bits = 0b%01d%01d%01d%01d_%01d%01d%01d%01d\r\n", 
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B7, 
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B6,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B5,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B4,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B3,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B2,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B1,
  //   TCA9535_Reg_map.PolarityInversion.Port.P1.bit.B0
  // );
}
