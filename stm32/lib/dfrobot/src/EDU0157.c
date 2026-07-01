
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sys_app.h"
#include "stm32wlxx_hal.h"
#include "i2c.h"

#include "EDU0157.h"


EDU0157_INTF_RET_TYPE EDU0157_i2c_recieve(uint8_t expected_cmd,char *out,
                                 size_t max_len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    uint8_t hdr[4];
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)dev_addr <<1,
                                                hdr, 4, i2c_timeout);
    if (status != HAL_OK) {
        return EDU0157_COMM_FAIL;
    }
    uint8_t res_status = hdr[0];
    uint8_t cmd    = hdr[1];
    uint16_t len   = hdr[2] | (hdr[3] << 8);

    if (res_status != 0x53)
        return EDU0157_STATUS_FAIL;

    if (cmd != expected_cmd)
        return EDU0157_MATCH_FAIL;

    if (len >= max_len)
        return EDU0157_OUTPUT_TOO_LONG;

    status = HAL_I2C_Master_Receive(
        &hi2c1,
        (uint16_t)dev_addr << 1,
        (uint8_t *)out,
        len,
        1000
    );
    if (status != HAL_OK)
    {
        return EDU0157_COMM_FAIL;
    }
    out[len] = '\0';

    return EDU0157_OK;
} 
EDU0157_INTF_RET_TYPE EDU0157_i2c_write(uint8_t cmd, const void *args, uint16_t arg_len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    uint8_t buf[64];
    if (arg_len > (sizeof(buf) - 3)) {
        return EDU0157_COMM_FAIL; 
    }
    buf[0] = cmd;
    buf[1] = arg_len & 0xFF;
    buf[2] = arg_len >> 8;

    if (arg_len)
        memcpy(&buf[3], args, arg_len);
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr << 1,
                                                 buf, arg_len+3, i2c_timeout);

    if (status != HAL_OK) {
        return EDU0157_COMM_FAIL;
    }

    return EDU0157_OK;
}


void EDU0157_delay_us(uint32_t period, void *intf_ptr)
{
    // calculate us to ms
    uint32_t period_ms = period / 1000;

    // if no period then set to 1 to ensure delay
    if (period_ms == 0) {
        period_ms = 1;
    }

    // delay
    HAL_Delay(period_ms);
}

int EDU0157_init(struct EDU0157_dev *dev) {
    if (!dev || !dev->read || !dev->write) return -1;
    return 0;
}


int EDU0157_get_value(struct EDU0157_dev *dev,const char *key,
                      char *out,
                      size_t max_len)
{
    dev->write(
        CMD_GET_DATA,
        key,
        strlen(key),
        dev->intf_ptr
    );

    dev->delay_us(50000,
        dev->intf_ptr);

    return dev->read(
        CMD_GET_DATA,
        out,
        max_len,
        dev->intf_ptr
    );
}