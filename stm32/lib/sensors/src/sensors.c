/**
 * @file sensors.c
 * @author John Madden (jmadden173@pm.me)
 * @brief See sensors.h
 * @date 2024-04-01
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

#include "sensors.h"

#include "userConfig.h"

/** Array for holding function callbacks */
static SensorsPrototypeMeasure callback_arr[MAX_SENSORS];

/** Length of @ref callback_arr */
static unsigned int callback_arr_len = 0;

static const uint8_t kBufferSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE;

/** Periodic timer for querying sensors */
static UTIL_TIMER_Object_t MeasureTimer;

static uint32_t measure_period = MEASUREMENT_PERIOD;

/**
 * @brief Measures sensors and adds to tx buffer
 *
 * Loops through all added callbacks and calls the functions. The resulting
 * serialized data is added to the buffer.
 */
void SensorsMeasure(void);

/**
 * @brief Runs the SensorsMeasure task
 *
 * The task priority is set to 0 to trigger the measurement of sensors.
 */
void SensorsRun(void);

void SensorsInit(void) {
  // set upload interval
  const UserConfiguration* cfg = UserConfigGet();
  // convert to ms
  measure_period = cfg->Upload_interval * 1000;

  // registers the task
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Measurement), UTIL_SEQ_RFU,
                   SensorsMeasure);

  // create the run timer
  UTIL_TIMER_Create(&MeasureTimer, measure_period, UTIL_TIMER_PERIODIC,
                    SensorsRun, NULL);
}

void SensorsStart(void) {
  // start the timer
  UTIL_TIMER_Start(&MeasureTimer);
}

void SensorsStop(void) {
  // stop the timer
  UTIL_TIMER_Stop(&MeasureTimer);
}

int SensorsAdd(SensorsPrototypeMeasure cb) {
  // check for out of range error
  if (callback_arr_len >= MAX_SENSORS) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error: Too many sensors added!\r\n");
    return -1;
  }

  // store callback in array
  callback_arr[callback_arr_len] = cb;

  // return index and increment
  return callback_arr_len++;
}

void SensorsMeasure(void) {
  // buffer to store measurements
  uint8_t buffer[kBufferSize];
  size_t buffer_len;

  // loop over callbacks
  for (int i = 0; i < callback_arr_len; i++) {
    // call measurement function
    buffer_len = callback_arr[i](buffer);
    APP_LOG(TS_ON, VLEVEL_M, "Callback index: %d\r\n", i);
    APP_LOG(TS_ON, VLEVEL_M, "Buffer length: %u\r\n", buffer_len);
    APP_LOG(TS_ON, VLEVEL_M, "Buffer: ");
    for (int j = 0; j < buffer_len; j++) {
      APP_LOG(TS_OFF, VLEVEL_M, "%x", buffer[j]);
    }
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

    // add to tx buffer
    FramStatus status = FramPut(buffer, buffer_len);
    if (status == FRAM_BUFFER_FULL) {
      APP_LOG(TS_OFF, VLEVEL_M, "Error: TX Buffer full!\r\n");
    } else if (status != FRAM_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "Error: General FRAM buffer!\r\n");
    }
  }
}

size_t SensorsMeasureTest(uint8_t* data) {
  uint8_t static_data[] = {0xa,  0xc,  0x8,  0xc8, 0x1,  0x10, 0xc8, 0x1,  0x18,
                           0x88, 0xba, 0xf3, 0xba, 0x6,  0x12, 0x9,  0x11, 0xd9,
                           0xce, 0xf7, 0x53, 0x3,  0x88, 0xb7, 0xc0};

  memcpy(data, static_data, sizeof(static_data));
  return sizeof(static_data);
}

void SensorsRun(void) {
  // trigger task to run
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Measurement), CFG_SEQ_Prio_0);
}
