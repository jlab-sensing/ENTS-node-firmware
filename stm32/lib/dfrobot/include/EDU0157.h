/** @file EDU0157.h
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
typedef struct {
    float wind_speed;   
    int32_t wind_direction; 
    float altitude;     
    float pressure;     
    float temperature;  
    float humidity;      
} EDU0157Data;
/*

reference:
https://github.com/DFRobot/DFRobot_LarkWeatherStation
bme280 library in this repo
*/
#define EDU0157_INTF_RET_TYPE                      int8_t
#define EDU0157_OK INT8_C(0)
#define EDU0157_COMM_FAIL INT8_C(-2)
#define EDU0157_STATUS_FAIL INT8_C(-1)
#define EDU0157_MATCH_FAIL INT8_C(-3)
#define EDU0157_OUTPUT_TOO_LONG INT8_C(-4)

#define CMD_GET_DATA                0x00 ///< Return the name based on the given name
#define CMD_GET_ALL_DATA            0x01 ///< Get all onboard sensor data
#define CMD_SET_TIME                0x02 ///< Set onboard RTC time
#define CME_GET_TIME                0x03
#define CMD_GET_UNIT                0x04 ///< Get sensor units
#define CMD_GET_VERSION             0x05 ///< Get version number
#define IIC_MAX_TRANSFER            32     ///< Maximum transferred data via I2C
#define I2C_ACHE_MAX_LEN            32

#define STATUS_SUCCESS      0x53  ///< Status of successful response   
#define STATUS_FAILED       0x63  ///< Status of failed response 


static const uint32_t i2c_timeout = 1000;

//copied this from bme280, kind of redundant.
typedef EDU0157_INTF_RET_TYPE (*EDU0157_read_fptr_t)(uint8_t expected_cmd,char *out,
                                 size_t max_len, void *intf_ptr);
typedef EDU0157_INTF_RET_TYPE (*EDU0157_write_fptr_t)(uint8_t cmd, const void *args, uint16_t arg_len, void *intf_ptr);


typedef void (*EDU0157_delay_us_fptr_t)(uint32_t period, void *intf_ptr);
EDU0157_INTF_RET_TYPE EDU0157_i2c_recieve(uint8_t expected_cmd,char *out,
                                 size_t max_len, void *intf_ptr);
EDU0157_INTF_RET_TYPE EDU0157_i2c_write(uint8_t cmd, const void *args, uint16_t arg_len, void *intf_ptr);
void EDU0157_delay_us(uint32_t period, void *intf_ptr);

/**
 * @brief EDU0157 Device structure
 * 
 */
struct EDU0157_dev
{
    void *intf_ptr;

    /*! Read function pointer */
    EDU0157_read_fptr_t read;

    /*! Write function pointer */
    EDU0157_write_fptr_t write;

    /*! Delay function pointer */
    EDU0157_delay_us_fptr_t delay_us;
};

/**
 * @brief Initialize the EDU0157 device structure
 * 
 * @param dev Pointer to the EDU0157 device structure
 * @return int 0 on success, -1 on failure
 */
int EDU0157_init(struct EDU0157_dev *dev);

/**
 * @brief Get a value from the EDU0157 device
 * 
 * @param dev Pointer to the EDU0157 device structure
 * @param key The key of the value to get *This is useless because they keys dont work so i always call get all data.
 * @param out Buffer to store the output value
 * @param max_len Maximum length of the output buffer
 * @return int 1 on success, 0 on failure
 */
int EDU0157_get_value(struct EDU0157_dev *dev,const char *key,
                      char *out,
                      size_t max_len);
