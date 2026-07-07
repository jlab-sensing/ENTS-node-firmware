//code heavily based off of existing cpp arduino library: https://github.com/adafruit/Adafruit_PM25AQI/
#ifndef PMSA003I_H
#define PMSA003I_H

#include <stdint.h>
#include <stdbool.h>

#include "sys_app.h"
#include "stm32wlxx_hal.h"
#include "i2c.h"


#define PMSA003I_I2C_ADDR             0x12  
#define PMSA003I_REG_START            0x00  
#define PMSA003I_BUFFER_SIZE          32    

typedef struct { //data structure inspired by: https://github.com/adafruit/Adafruit_PM25AQI/blob/master/src/Adafruit_PM25AQI.h
    uint16_t pm10_standard;   
    uint16_t pm25_standard;  
    uint16_t pm100_standard;  
    
    uint16_t pm10_env;      
    uint16_t pm25_env;       
    uint16_t pm100_env;      
    
    uint16_t particles_03um; 
    uint16_t particles_05um; 
    uint16_t particles_10um; 
    uint16_t particles_25um; 
    uint16_t particles_50um; 
    uint16_t particles_100um;
} pmsa003i_data_t;

bool pmsa003i_read(pmsa003i_data_t *out_data);

#endif