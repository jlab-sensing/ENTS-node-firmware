#include "EDU0157_sensor.h"


/**
 * @brief Required time between measurements
 * 
 * @see BME280Init
 */
static uint32_t period = 0;

/**
 * @brief Device definition
 * 
 * @see BME280Init
 */
static struct EDU0157_dev dev;
static uint8_t dev_addr=DFR_DEVICE_ADDR;


int EDU0157Init(void) {
    dev_addr = DFR_DEVICE_ADDR;
    dev.read = EDU0157_i2c_recieve;
    dev.write = EDU0157_i2c_write;
    dev.delay_us = EDU0157_delay_us;
    dev.intf_ptr = &dev_addr;
    if (EDU0157_init(&dev) != 0)
    {
        return 0;
    }
  return 1;
}
size_t EDU0157Measure(uint8_t *data, SysTime_t ts, uint32_t idx)
{
    char value[64];

    if (EDU0157_get_value(&dev, "temp", value, sizeof(value)) != EDU0157_OK)
    {
        return 0;
    }

    size_t len = strlen(value);

    memcpy(data, value, len);

    return len;
}