/**
 * @file sensors.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Queries sensors and adds measurements to the upload queue
 * @date 2024-04-01
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
 * MEASUREMENT_PERIOD defines the amount of time between measurements. This
 * value should be an order of magnitude greater than the upload frequency that
 * is defined by APP_TX_DUTY_CYCLE.
 *
 * @copyright
 *
 * MIT License
 *
 * Copyright (c) 2024 jLab in Smart Sensing at UCSC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LIB_SENSORS_INCLUDE_SENSORS_H_
#define LIB_SENSORS_INCLUDE_SENSORS_H_

#include "fifo.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "sys_app.h"

#ifdef __cplusplus
extern "C" {
#endif

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
typedef size_t (*SensorsPrototypeMeasure)(uint8_t* data);

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

#ifdef __cplusplus
}
#endif

#endif  // LIB_SENSORS_INCLUDE_SENSORS_H_
