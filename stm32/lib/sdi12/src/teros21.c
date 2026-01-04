#include "teros21.h"

#include "sensor.h"
#include "sensors.h"
#include "userConfig.h"

SDI12Status Teros21ParseMeasurement(const char *buffer, Teros21Data *data) {
  char addr = 0;
  float matric_pot = 0.;
  float temp = 0.;

  // parse string
  int rc = sscanf(buffer, "%1c-%f%f", &addr, &matric_pot, &temp);
  if (rc < 3) {
    return SDI12_PARSING_ERROR;
  }

  // assign data to struct
  data->addr = addr;
  data->matric_pot = -1 * matric_pot;
  data->temp = temp;

  return SDI12_OK;
}

SDI12Status Teros21GetMeasurement(char addr, Teros21Data *data) {
  // buffer to store measurement
  char buffer[18];

  // status messages
  SDI12Status status = SDI12_OK;

  // get measurement string
  // Measured 130ms experimentally, set to 200 ms to be safe
  SDI12_Measure_TypeDef measurment_info;
  status = SDI12GetMeasurment((uint8_t)addr, &measurment_info, buffer, 1000);
  if (status != SDI12_OK) {
    return status;
  }

  // parse measurement into data structure
  status = Teros21ParseMeasurement(buffer, data);
  if (status != SDI12_OK) {
    return status;
  }

  return status;
}

size_t Teros21Measure(uint8_t *data, SysTime_t ts, uint32_t idx) {
  Teros21Data sens_data = {};
  SDI12Status status = Teros21GetMeasurement('0', &sens_data);
  if (status != SDI12_OK) {
    return -1;
  }

  const UserConfiguration *cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  size_t data_len = 0;
  SensorStatus sen_status = SENSOR_OK;

  // matric potential
  sen_status =
      EncodeDoubleMeasurement(meta, sens_data.matric_pot,
                              SensorType_TEROS21_MATRIC_POT, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // temperature
  sen_status = EncodeDoubleMeasurement(
      meta, sens_data.temp, SensorType_TEROS21_TEMP, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }

  return data_len;
}
