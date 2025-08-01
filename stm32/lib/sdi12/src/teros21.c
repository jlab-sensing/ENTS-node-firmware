#include "teros21.h"

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
  size_t buffer_len = 5;
  status = Teros21ParseMeasurement(buffer, data);
  if (status != SDI12_OK) {
    return status;
  }

  return status;
}

size_t Teros21Measure(uint8_t *data, SysTime_t ts) {
  Teros21Data sens_data = {};
  SDI12Status status = Teros21GetMeasurement('0', &sens_data);
  if (status != SDI12_OK) {
    return -1;
  }

  const UserConfiguration *cfg = UserConfigGet();

  size_t data_len =
      EncodeTeros21Measurement(ts.Seconds, cfg->logger_id, cfg->cell_id,
                               sens_data.matric_pot, sens_data.temp, data);

  return data_len;
}
