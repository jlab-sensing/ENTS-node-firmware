/**
 * @file    ads.c
 * @author  Stephen Taylor
 * @brief   Soil Power Sensor ADS12 library
 * @date    11/27/2023
 *
 * This file provides a function to read from the ADS1219.
 *
 * TODO (jmadden173): Improve lib with big bang for regs
 * TODO (jmadden173): Add offset calibration at startup
 **/

#include "ads.h"

#include <stm32wlxx_hal_gpio.h>

#include "sensor.h"
#include "userConfig.h"

/** i2c address */
static const uint8_t addr = 0x40;
/** i2c address left shifted one bit for hal i2c funcs */
static const uint8_t addrls = addr << 1;

/** Reset the device */
static const uint8_t cmd_reset = 0x06;
/** Start or restart conversion */
static const uint8_t cmd_start = 0x08;
/** Enter power-down mode */
static const uint8_t cmd_powerdown = 0x02;
/** Read data */
static const uint8_t cmd_rdata = 0x10;
/** Write to register */
// prev 0x80
static const uint8_t cmd_rreg = 0x20;
/** Read from register */
static const uint8_t cmd_wreg = 0x40;

// #define CALIBRATION
#define DELTA_ENCODING
int32_t previous_voltage_reading = 0;
int32_t previous_current_reading = 0;

// default sane calibration
static double voltage_calibration_m = 0.0;
static double voltage_calibration_b = 0.0;
static double current_calibration_m = 0.0;
static double current_calibration_b = 0.0;

/**
 * Control register breakdown.
 *
 * The implementation for the bit field uses the first value as the LSB. Aka
 * vref is the LSB and mux is the MSB.
 *
 *  7:5 MUX (default)
 *  4   Gain (default)
 *  3:2 Data rate (default)
 *  1   Conversion mode (default)
 *  0   VREF (External reference 3.3V)
 */
typedef union {
  uint8_t value;
  struct {
    uint8_t vref : 1;
    uint8_t mode : 1;
    uint8_t dr : 2;
    uint8_t gain : 1;
    uint8_t mux : 3;
  } bits;
} ConfigReg;

/**
 * @brief GPIO port for adc data ready line
 *
 * @see data_ready_pin
 */
const GPIO_TypeDef *data_ready_port = GPIOC;

/**
 * @brief GPIO pin for adc data ready line
 *
 */
const uint16_t data_ready_pin = GPIO_PIN_0;

/** Uart timeout in ms */
static const unsigned int g_timeout = 5000;

/**
 * @brief Turn on power to analog circuit
 *
 * Has a blocking wait of 1 mS to account for the startup time of OpAmps. This
 * should be an order of magnitude greater than the startup or turn-on time of
 * the opamps used in the circuit.
 *
 * MAX9944 - N/A
 * INA296 - 20 uS
 * THS4532 - 420 nS
 *
 * @see data_ready_pin
 */
void PowerOn(void);

/**
 * @brief Turn off power to analog circuit
 *
 * @see data_ready_pin
 */
void PowerDown(void);

/**
 * @brief Measure from the adc
 *
 * @param mwas Raw measurement
 *
 * @return Raw measurement from adc
 */
HAL_StatusTypeDef Measure(int32_t *meas);

/**
 * @brief This function reconfigures the ADS1219 based on the parameter reg_data
 *
 * @param reg_data
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef Configure(const ConfigReg reg_data);

HAL_StatusTypeDef ADC_init(void) {
  const UserConfiguration *cfg = UserConfigGet();

  // read calibration values
  voltage_calibration_m = cfg->Voltage_Slope;
  voltage_calibration_b = cfg->Voltage_Offset;
  current_calibration_m = cfg->Current_Slope;
  current_calibration_b = cfg->Current_Offset;

  HAL_StatusTypeDef ret = HAL_OK;

  // Send the reset code
  ret = HAL_I2C_Master_Transmit(&hi2c2, addrls, &cmd_reset, 1, g_timeout);
  if (ret != HAL_OK) {
    return ret;
  }

  // wait minimum 500 us to reach steady state
  HAL_Delay(1);

  return ret;
}

HAL_StatusTypeDef Configure(const ConfigReg reg_data) {
  HAL_StatusTypeDef ret;
  const uint8_t i2c_data[2] = {cmd_wreg, reg_data.value};
  ret = HAL_I2C_Master_Transmit(&hi2c2, addrls, i2c_data, sizeof(i2c_data),
                                g_timeout);
  return ret;
}

double ADC_readVoltage(void) {
  HAL_StatusTypeDef ret = HAL_OK;
  int32_t raw = 0;
  double meas = 0.0;

  ConfigReg reg_data = {0};
  reg_data.bits.vref = 1;

  // 0x01 is single shot 0x03 is continuous
  ret = Configure(reg_data);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

  ret = Measure(&raw);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

#ifdef CALIBRATION
  meas = (double)raw;
#else
  meas = (voltage_calibration_m * raw) + voltage_calibration_b;
#endif

  meas /= 1000;

  return meas;
}

double ADC_readCurrent(void) {
  HAL_StatusTypeDef ret = HAL_OK;
  int32_t raw = 0;
  double meas = 0.0;

  ConfigReg reg_data = {0};
  reg_data.bits.mux = 0b001;
  reg_data.bits.vref = 1;

  // 0x21 is single shot and 0x23 is continuos
  ret = Configure(reg_data);  // configure to read current
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

  ret = Measure(&raw);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

#ifdef CALIBRATION
  meas = (double)raw;
#else
  meas = (current_calibration_m * raw) + current_calibration_b;
#endif

  return meas;
}

HAL_StatusTypeDef probeADS12(void) {
  HAL_StatusTypeDef ret;
  ret = HAL_I2C_IsDeviceReady(&hi2c2, addrls, 10, 20);
  return ret;
}

size_t ADC_measureVoltage(uint8_t *data, SysTime_t ts, uint32_t idx) {
  double voltage = ADC_readVoltage();

  const UserConfiguration *cfg = UserConfigGet();

  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  size_t data_len = 0;

  SensorStatus status = SENSOR_OK;
  status = EncodeDoubleMeasurement(meta, voltage, SensorType_POWER_VOLTAGE,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  return data_len;
}

size_t ADC_measureCurrent(uint8_t *data, SysTime_t ts, uint32_t idx) {
  double voltage = ADC_readCurrent();

  const UserConfiguration *cfg = UserConfigGet();

  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  size_t data_len = 0;

  SensorStatus status = SENSOR_OK;
  status = EncodeDoubleMeasurement(meta, voltage, SensorType_POWER_CURRENT,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  return data_len;
}

void PowerOn(void) {
  // set high
  HAL_GPIO_WritePin(POWERDOWN_GPIO_Port, POWERDOWN_Pin, GPIO_PIN_SET);
  // delay for settling of analog components
  HAL_Delay(1);
}

void PowerOff(void) {
  // set low
  HAL_GPIO_WritePin(POWERDOWN_GPIO_Port, POWERDOWN_Pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef Measure(int32_t *meas) {
  HAL_StatusTypeDef ret = HAL_OK;
  uint8_t rx_data[3] = {0x00, 0x00, 0x00};

  PowerOn();

  // start conversion
  HAL_I2C_Master_Transmit(&hi2c2, addrls, &cmd_start, 1, g_timeout);

  // wait for conversion
  HAL_Delay(60);

  // Wait for the DRDY pin on the ADS12 to go low, this means data is ready
  while (HAL_GPIO_ReadPin(data_ready_port, data_ready_pin)) {
  }

  // send read data command
  ret = HAL_I2C_Master_Transmit(&hi2c2, addrls, &cmd_rdata, 1, g_timeout);
  if (ret != HAL_OK) {
    return -1;
  }

  // read 3 bytes of data
  ret = HAL_I2C_Master_Receive(&hi2c2, addrls, rx_data, 3, g_timeout);
  if (ret != HAL_OK) {
    return -1;
  }

  PowerOff();

  // Combine the 3 bytes into a 24-bit value
  *meas = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) |
          ((int32_t)rx_data[2]);
  // Check if the sign bit (24th bit) is set
  if (*meas & 0x800000) {
    // Extend the sign to 32 bits
    *meas |= 0xFF000000;
  }

  return ret;
}
