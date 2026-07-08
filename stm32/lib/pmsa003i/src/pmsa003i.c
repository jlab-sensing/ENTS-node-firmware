#include "pmsa003i.h"
//based off bme280_common

// system includes
#include "sys_app.h"
#include "stm32_systime.h"

// user includes
#include "transcoder.h"
#include "userConfig.h"
#include "sensor.h"
#include "sensors.h"


static bool i2c_read(uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t rx_len) {

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, 
                                                (PMSA003I_I2C_ADDR << 1), tx[0], I2C_MEMADD_SIZE_8BIT, 
                                                rx, rx_len, 100);
    if (status == HAL_OK){
    return true; 
    }
    else{
        HAL_I2C_Init(&hi2c1);
        return false;
    }
}


bool pmsa003i_read( pmsa003i_data_t *out_data) {
    uint8_t start_reg = PMSA003I_REG_START;
    uint8_t buffer[PMSA003I_BUFFER_SIZE] = {0};

    if ( out_data == NULL) {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "data pointer not ok\r\n");
        return false;
    }
    if (!i2c_read(&start_reg, 1, buffer, PMSA003I_BUFFER_SIZE)) {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "hal not ok\r\n");
        return false; 
    }

    // following code is adapted from adafruit library

    if (buffer[0] != 0x42 || buffer[1] != 0x4D) {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "data not ok - pmsa003i\r\n");
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Raw buffer: ");
        for (int i = 0; i < PMSA003I_BUFFER_SIZE; i++) {
            APP_LOG(TS_OFF, VLEVEL_ALWAYS, "%02X ", buffer[i]);
        }
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "\r\n");
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
        for (int i = 0; i < PMSA003I_BUFFER_SIZE; i++) {
            APP_LOG(TS_OFF, VLEVEL_ALWAYS, "%02X ", buffer[i]);
        }
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "\r\n");
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Converted buffer: ");
        for (int i = 0; i < 15; i++) {
            APP_LOG(TS_OFF, VLEVEL_ALWAYS, "%04X ", buffer_u16[i]);
        }
        APP_LOG(TS_OFF, VLEVEL_ALWAYS, "\r\n");
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

size_t PMSA003IMeasure(uint8_t *data, SysTime_t ts, uint32_t idx){
    pmsa003i_data_t sens_data = {};
  bool status = pmsa003i_read(&sens_data);
  if (status != true) {
    return -1;
  }

  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  SensorStatus sen_status = SENSOR_OK;
  size_t data_len = 0;

// stadnard pm
sen_status = EncodeUint32Measurement(meta, sens_data.pm10_standard, SensorType_PMSA003I_PM1_0_STD, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.pm25_standard, SensorType_PMSA003I_PM2_5_STD, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.pm100_standard, SensorType_PMSA003I_PM10_0_STD, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);


// env pm
sen_status = EncodeUint32Measurement(meta, sens_data.pm10_env, SensorType_PMSA003I_PM1_0_ENV, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.pm25_env, SensorType_PMSA003I_PM2_5_ENV, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.pm100_env, SensorType_PMSA003I_PM10_0_ENV, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);


// particle cts
sen_status = EncodeUint32Measurement(meta, sens_data.particles_03um, SensorType_PMSA003I_PARTICLES_0_3UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.particles_05um, SensorType_PMSA003I_PARTICLES_0_5UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.particles_10um, SensorType_PMSA003I_PARTICLES_1_0UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.particles_25um, SensorType_PMSA003I_PARTICLES_2_5UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.particles_50um, SensorType_PMSA003I_PARTICLES_5_0UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

sen_status = EncodeUint32Measurement(meta, sens_data.particles_100um, SensorType_PMSA003I_PARTICLES_10_0UM, data, &data_len);
if (sen_status != SENSOR_OK) return -1;
SensorsAddMeasurement(data, data_len);

  return data_len;
}