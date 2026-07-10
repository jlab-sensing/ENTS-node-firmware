/** @file EDU0157_sensor.h
 * 
 * @brief EDU0157 i2c library
 * 
 * Based on dfrobot library: https://github.com/DFRobot/DFRobot_LarkWeatherStation
 * 
 * @author Kai Otsuka
 * @date 2026-7-7
 * 
 * 
*/
#include <stdint.h>
#include <stddef.h>
// system includes
#include "sys_app.h"
#include "stm32_systime.h"

// user includes
#include "EDU0157.h"
#include "transcoder.h"
#include "userConfig.h"
#include "sensor.h"
#include "sensors.h"


#define DFR_DEVICE_ADDR                  UINT8_C(0x42)

/**
 * @brief Initialize the EDU0157 sensor
 * 
 * @return int 1 on success, 0 on failure
 * @param void
 */
int EDU0157Init(void);


/**
 * @brief Measure all sensor data from the EDU0157 sensor
 * 
 * @param uint8_t *data Buffer to store the sensor data
 * @param SysTime_t ts Timestamp of the measurement
 * @param uint32_t idx Index of the measurement
 * @return size_t,-1 if failed
 */
size_t EDU0157Measure(uint8_t *data, SysTime_t ts, uint32_t idx);


/**
 * @brief Measure all sensor data from the EDU0157 sensor and store it in the provided structure
 * 
 * @param EDU0157Data *sensor_data Pointer to the structure to store the sensor data
 * @return int 1 on success, -1 on failure
 */
int EDU0157MeasureAll(EDU0157Data *sensor_data);