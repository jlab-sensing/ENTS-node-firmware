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
int EDU0157Init(void);
size_t EDU0157Measure(uint8_t *data, SysTime_t ts, uint32_t idx);
int EDU0157MeasureAll(EDU0157Data *sensor_data);