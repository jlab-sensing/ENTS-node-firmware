#include "payload.h"

#include "fifo.h"
#include "sensor.h"

PayloadStatus FormatPayload(uint8_t* buffer, size_t size, size_t* length) {
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
  while (current_payload_size <= size) {
    // get next serialized measurement
    fram_status = FramPeek(meas_count, buffer, (uint8_t*) length);
    if (fram_status == FRAM_BUFFER_EMPTY || fram_status == FRAM_OUT_OF_RANGE) {
      // no more data to read
      if (meas_count == 0) {
        return PAYLOAD_NO_DATA;
      } else {
        break;
      }
    } else if (fram_status != FRAM_OK) {
      APP_LOG(TS_ON, VLEVEL_M,
              "Error peeking data from fram buffer. FramStatus = %d\r\n", fram_status);
      return PAYLOAD_ERROR;
    }

    // decode measurement
    sensor_status = DecodeSensorMeasurement(buffer, *length, &meas[meas_count++]);
    if (sensor_status != SENSOR_OK) {
      APP_LOG(TS_ON, VLEVEL_M,
              "Error decoding sensor measurement from buffer. SensorStatus = %d\r\n", sensor_status);
      return PAYLOAD_ERROR;
    }

    sensor_status = RepeatedSensorMeasurementsSize(meta, meas,
        meas_count, &current_payload_size); 
    if (sensor_status != SENSOR_OK) {
      APP_LOG(TS_ON, VLEVEL_M,
              "Error calculating repeated sensor measurements size. SensorStatus = %d\r\n", sensor_status);
      return PAYLOAD_ERROR;
    }
  }

  // when exit we are over the payload limit so back off one
  //meas_count--;

  // encode measurements into AppData buffer
  EncodeRepeatedSensorMeasurements(meta, meas,
      meas_count, buffer, size, length);

  // drop uploaded measurements
  for (int i = 0; i < meas_count; i++) {
    FramDrop();
  }
}
