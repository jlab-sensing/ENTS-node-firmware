/**
 * @file sensors.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Queries sensors and adds measurements to the upload queue
 * @date 2024-04-01
 */

#ifndef LIB_SENSORS_INCLUDE_SENSORS_H_
#define LIB_SENSORS_INCLUDE_SENSORS_H_

#include "fifo.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_systime.h"
#include "stm32_timer.h"
#include "sys_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup stm32
 * @defgroup sensors Sensors
 * @brief Sensors library provides the interface between taking measurements
 * and uploading them to the server
 *
 * Provides an interface for querying sensors and adding the measurements to the
 * transmit buffer. Functions to query sensors are registered and called on a
 * interval that is global to all sensors.
 *
 * The library expects all initialization code for registered sensors to be
 * called before SensorsStart. The timer utility library (UTIL_TIMER_Init) and
 * sequencer utility (UTIL_SEQ_Init) must be initialized as well.
 *
 * Currently there is no way to modify or remove sensors after adding the
 * callback function. This is based on the assumption sensors are not going
 * to be dynamically added or removed during firmware runtime. Also new sensors
 * will require updates to the firmware binaries.
 *
 * The measurement interval is determined by the user. This value should be an
 * order of magnitude greater than the upload frequency that is defined by
 * APP_TX_DUTY_CYCLE.
 *
 * @{
 */

#ifndef MAX_SENSORS
/** Max number of sensors that can be added*/
#define MAX_SENSORS 32
#endif /* MAX_SENSORS */

#ifndef MEASUREMENT_PERIOD
/** The amount of time between measurements */
#define MEASUREMENT_PERIOD 15000
#endif /* MEASUREMENT_PERIOD */

/**
 * @brief Function prototype for measure functions
 *
 * @param data Location to store data
 *
 * @return Number of bytes in data
 */
typedef size_t (*SensorsPrototypeMeasure)(uint8_t *data, SysTime_t ts);

/**
 * @brief Registers the measurement task with the sequencer
 *
 */
void SensorsInit(void);

/**
 * @brief Starts timer to take measurements and add to upload queue
 *
 */
void SensorsStart(void);

/**
 * @brief Stops measurement timer
 *
 */
void SensorsStop(void);

/**
 * @brief Adds sensor call to measurement cycle
 *
 * @param cb Callback to the measurement function
 *
 * @return Index of callback in internal array, -1 indicates an error
 */
int SensorsAdd(SensorsPrototypeMeasure cb);

/**
 * @brief Function for adding static test measurements
 *
 * Uses adc measurements
 * a c 8 c8 1 10 c8 1 18 88 ba f3 ba 6 12 9 11 d9 ce f7 53 3 88 b7 c0
 *
 * @see SensorsPrototypeMeasure
 */
size_t SensorsMeasureTest(uint8_t *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_SENSORS_INCLUDE_SENSORS_H_
