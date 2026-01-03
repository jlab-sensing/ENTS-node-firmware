#include "payload.h"

#include "fifo.h"
#include "sensor.h"

PayloadStatus FormatPayload(uint8_t max_size, uint8_t* buffer,
    uint8_t* size) {
  // array of measurements that will get uploaded
  SensorMeasurement meas[16] = {};
  size_t meas_count = 0;

  // storing current payload size
  size_t current_payload_size = 0;

  FramStatus fram_status = FRAM_OK;
  SensorStatus sensor_status = SENSOR_OK;


  // NOTE: This is a temp fix for the function input. Ideally the metadata
  // would be automatically handled and optimized for packet size.
  Metadata meta = Metadata_init_default;

  // Loop until the payload size is exceeded
  while (current_payload_size <= max_size) {
    // buffer to store encoded measurements
    uint8_t meas_buffer[256] = {};
    uint8_t meas_buffer_size = 0;

    // get next serialized measurement
    fram_status = FramPeek(meas_count, meas_buffer, &meas_buffer_size);
    if (fram_status != FRAM_OK) {
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error peeking data from fram buffer. FramStatus = %d", fram_status);
      break;
    }

    // decode measurement
    sensor_status = DecodeSensorMeasurement(meas_buffer, meas_buffer_size, &meas[meas_count++]);
    if (sensor_status != SENSOR_OK) {
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error decoding sensor measurement from buffer. SensorStatus = %d", sensor_status);
      break;
    }

    current_payload_size = RepeatedSensorMeasurementsSize(meta, meas,
        meas_count, &current_payload_size); 
  }

  // when exit we are over the payload limit so back off one
  meas_count--;

  // encode measurements into AppData buffer
  EncodeRepeatedSensorMeasurements(meta, meas,
      meas_count, buffer, size);

  // drop uploaded measurements
  for (int i = 0; i < meas_count; i++) {
    FramDrop();
  }
}
