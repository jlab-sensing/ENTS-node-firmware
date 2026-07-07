#include "pmsa003i.h"
//based off bme280_common

static bool i2c_read(uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t rx_len) {

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, 
                                                (PMSA003I_I2C_ADDR << 1), tx[0], I2C_MEMADD_SIZE_8BIT, 
                                                rx, rx_len, 100);
    return (status == HAL_OK);
    return true; 
}


bool pmsa003i_read( pmsa003i_data_t *out_data) {
    uint8_t start_reg = PMSA003I_REG_START;
    uint8_t buffer[PMSA003I_BUFFER_SIZE] = {0};

    if ( out_data == NULL) return false;

    if (!i2c_read(&start_reg, 1, buffer, PMSA003I_BUFFER_SIZE)) {
        return false; 
    }

    // following code is adapted from adafruit library

    if (buffer[0] != 0x42 || buffer[1] != 0x4D) {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "data not ok - pmsa003i\r\n");
        return false; 
    }

    uint16_t sum = 0;
    for (int i = 0; i < 30; i++) {
        sum += buffer[i];
    }
    
    // data is endianed so this fixes it
    uint16_t buffer_u16[15];
    for (uint8_t i = 0; i < 15; i++) {
        buffer_u16[i] = buffer[2 + i * 2 + 1];
        buffer_u16[i] += (buffer[2 + i * 2] << 8);
    }


    uint16_t checksum = buffer_u16[14];
    if (sum != checksum) {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "checksum doesn't match\r\n");
        return false; 
    }

    out_data->pm10_standard   = buffer_u16[1];
    out_data->pm25_standard   = buffer_u16[2];
    out_data->pm100_standard  = buffer_u16[3];

    out_data->pm10_env        = buffer_u16[4];
    out_data->pm25_env        = buffer_u16[5];
    out_data->pm100_env       = buffer_u16[6];

    out_data->particles_03um  = buffer_u16[7];
    out_data->particles_05um  = buffer_u16[8];
    out_data->particles_10um  = buffer_u16[9];
    out_data->particles_25um  = buffer_u16[10];
    out_data->particles_50um  = buffer_u16[11];
    out_data->particles_100um = buffer_u16[12];

    return true;
}