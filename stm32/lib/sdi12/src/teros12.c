#include "teros12.h"

#include "sensor.h"
#include "sensors.h"
#include "userConfig.h"

SDI12Status Teros12ParseMeasurement(const char *buffer, Teros12Data *data) {
  // parse string and check number of characters parsed
  int rc = sscanf(buffer, "%1c+%f%f+%d", &data->addr, &data->vwc, &data->temp,
                  &data->ec);
  if (rc < 4) {
    return SDI12_PARSING_ERROR;
  }

  return SDI12_OK;
}

SDI12Status Teros12GetMeasurement(char addr, Teros12Data *data) {
  // buffer to store measurement
  // based on the measurement range of the device the max string length is
  // 0+1846.16+22.3+20000 = 20
  char buffer[20];

  // status messages
  SDI12Status status = SDI12_OK;

  // get measurement string
  // Measured 130ms experimentally, set to 200 ms to be safe
  SDI12_Measure_TypeDef measurement_info;
  status = SDI12GetMeasurement((uint8_t)addr, &measurement_info, buffer, 1000);
  if (status != SDI12_OK) {
    return status;
  }

  // parse measurement into data structure
  status = Teros12ParseMeasurement(buffer, data);
  if (status != SDI12_OK) {
    return status;
  }

  return status;
}

size_t Teros12Measure(uint8_t *data, SysTime_t ts, uint32_t idx) {
  Teros12Data sens_data = {};
  SDI12Status status = Teros12GetMeasurement('0', &sens_data);
  APP_LOG(TS_ON, VLEVEL_H,
          "\tTeros12GetMeasurement() return %d (vwc=%f, temp=%f, ec=%d)\r\n",
          status, sens_data.vwc, sens_data.temp, sens_data.ec);
  if (status != SDI12_OK) {
    return -1;
  }

  const UserConfiguration *cfg = UserConfigGet();

  // calibration equation for mineral soils from Teros12 user manual and scale
  // to percent scale
  // https://publications.metergroup.com/Manuals/20587_TEROS11-12_Manual_Web.pdf?_gl=1*174xdyp*_gcl_au*MTIxODkwMzcuMTc0MTIwMjU3Nw..
  float vwc_adj = (3.879e-4 * sens_data.vwc) - 0.6956;
  vwc_adj *= 100;
  APP_LOG(TS_ON, VLEVEL_H, "\tvwc_adj == %f \r\n", vwc_adj);

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  // variables for the next block
  size_t data_len = 0;
  SensorStatus sen_status = SENSOR_OK;

  // vwc
  sen_status = EncodeDoubleMeasurement(meta, sens_data.vwc,
                                       SensorType_TEROS12_VWC, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // vwc_adj
  sen_status = EncodeDoubleMeasurement(
      meta, vwc_adj, SensorType_TEROS12_VWC_ADJ, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // temp
  sen_status = EncodeDoubleMeasurement(
      meta, sens_data.temp, SensorType_TEROS12_TEMP, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // ec
  sen_status = EncodeUint32Measurement(meta, sens_data.ec,
                                       SensorType_TEROS12_EC, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }

  return data_len;
}
