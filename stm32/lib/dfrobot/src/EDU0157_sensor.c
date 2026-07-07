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
static int direction_to_int(const char *dir)
{
    if (strcmp(dir, "N")  == 0) return 0;
    if (strcmp(dir, "NE") == 0) return 1;
    if (strcmp(dir, "E")  == 0) return 2;
    if (strcmp(dir, "SE") == 0) return 3;
    if (strcmp(dir, "S")  == 0) return 4;
    if (strcmp(dir, "SW") == 0) return 5;
    if (strcmp(dir, "W")  == 0) return 6;
    if (strcmp(dir, "NW") == 0) return 7;

    return -1;  
}

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
int EDU0157MeasureAll(EDU0157Data *sensor_data)
{
    char value[300];

    if (EDU0157_get_value(&dev, "temp", value, sizeof(value)) != 1)
    {
        return 0;
    }

    char direction[4];

    int ret = sscanf(
        value,
        "WindSpeed:%f m/s, WindDirection:%3[^,], Altitude:%f m, Pressure:%f hPa, Temp:%f C, Humi:%f %%RH",
        &sensor_data->wind_speed,
        direction,
        &sensor_data->altitude,
        &sensor_data->pressure,
        &sensor_data->temperature,
        &sensor_data->humidity
    );

    if (ret != 6)
        return -1;

    sensor_data->wind_direction = direction_to_int(direction);

    return 1;  
}
size_t EDU0157Measure(uint8_t *data, SysTime_t ts, uint32_t idx)
{
    
    EDU0157Data sensor_data = {};
    int status  = EDU0157MeasureAll(&sensor_data);
    if (status <= 0)
    {
        return -1;
    }
    const UserConfiguration* cfg = UserConfigGet();

    // metadata
    Metadata meta = Metadata_init_zero;
    meta.ts = ts.Seconds;
    meta.logger_id = cfg->logger_id;
    meta.cell_id = cfg->cell_id;

    SensorStatus sen_status;
    size_t data_len = 0;


    sen_status = EncodeDoubleMeasurement(
        meta, sensor_data.wind_speed, SensorType_EDU0157_WIND_SPEED, data, &data_len);
    if (sen_status != SENSOR_OK) {
        return -1;
    }
    SensorsAddMeasurement(data, data_len);
    //wd
    sen_status = EncodeUint32Measurement(
        meta,
        sensor_data.wind_direction,
        SensorType_EDU0157_WIND_DIRECTION,
        data,
        &data_len);
    if (sen_status != SENSOR_OK)
        return -1;
    SensorsAddMeasurement(data, data_len);

    //alttideu
    sen_status = EncodeDoubleMeasurement(
        meta,
        sensor_data.altitude,
        SensorType_EDU0157_ALTITUDE,
        data,
        &data_len);
    if (sen_status != SENSOR_OK)
        return -1;
    SensorsAddMeasurement(data, data_len);

    //pressure
    sen_status = EncodeDoubleMeasurement(
        meta,
        sensor_data.pressure,
        SensorType_EDU0157_PRESSURE,
        data,
        &data_len);
    if (sen_status != SENSOR_OK)
        return -1;
    SensorsAddMeasurement(data, data_len);

    //temp
    sen_status = EncodeDoubleMeasurement(
        meta,
        sensor_data.temperature,
        SensorType_EDU0157_TEMP,
        data,
        &data_len);
    if (sen_status != SENSOR_OK)
        return -1;
    SensorsAddMeasurement(data, data_len);

    //relative humidity
    sen_status = EncodeDoubleMeasurement(
        meta,
        sensor_data.humidity,
        SensorType_EDU0157_HUMIDITY,
        data,
        &data_len);
    if (sen_status != SENSOR_OK)
        return -1;
    SensorsAddMeasurement(data, data_len);
    return data_len;
}
