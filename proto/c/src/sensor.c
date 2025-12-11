#include "sensor.h"

#include "pb_decode.h"
#include "pb_encode.h"


bool MetadataEqual(Metadata* left, Metadata* right) {
    if (left->ts != right->ts) {
        return false;
    }
    if (left->logger_id != right->logger_id) {
        return false;
    }
    if (left->cell_id != right->cell_id) {
        return false;
    }

    return true;
}

SensorStatus EncodeSensorMeasurement(const SensorMeasurement *meas, uint8_t *buffer, size_t *size) {
    // create output stream
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, *size);
    // encode message and check rc
    bool status = pb_encode(&ostream, SensorMeasurement_fields, meas);
    if (!status) {
        return SENSOR_ERROR;
    }

    // return number of bytes written
    *size = ostream.bytes_written;
    return SENSOR_OK;
}


SensorStatus EncodeRepeatedSensorMeasurements(Metadata meta, const SensorMeasurement meas[],
    size_t count, uint8_t* buffer, size_t* size) {

    RepeatedSensorMeasurements rep_meas = RepeatedSensorMeasurements_init_zero;

    size_t max_count = sizeof(rep_meas.measurements) / sizeof(rep_meas.measurements[0]);
   
    // check count doesn't exceed max count
    if (count > max_count) {
        return SENSOR_OUT_OF_BOUNDS;
    }

    // handle metadata
    if (MetadataEqual(&meta, &METADATA_NONE)) {
        rep_meas.has_meta = false;
    } else {
        rep_meas.has_meta = true;
        rep_meas.meta = meta;
    }

    // copy measurements
    rep_meas.measurements_count = count;
    for (size_t i = 0; i < count; i++) {
        rep_meas.measurements[i] = meas[i];
    }

    // encode
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, *size);
    bool status = pb_encode(&ostream, RepeatedSensorMeasurements_fields, &rep_meas);
    if (!status) {
        return SENSOR_ERROR;
    }

    *size = ostream.bytes_written;
    return SENSOR_OK;
}


SensorStatus EncodeUint64Measurement(Metadata meta,  uint64_t value, SensorType type,
                             uint8_t* buffer, size_t* size) {
    SensorMeasurement meas = SensorMeasurement_init_zero;
 
    // has_meta is handled in EncodeMeasuremnet function
    meas.meta = meta;
    meas.has_meta = true;

    meas.type = type;
    meas.which_value = SensorMeasurement_unsigned_int_tag;
    meas.value.unsigned_int = value;

    return EncodeSensorMeasurement(&meas, buffer, size);
}

SensorStatus EncodeInt64Measurement(Metadata meta, int64_t value, SensorType type,
                            uint8_t* buffer, size_t* size) {
    SensorMeasurement meas = SensorMeasurement_init_zero;

    meas.meta = meta;
    meas.has_meta = true;

    meas.type = type;
    meas.which_value = SensorMeasurement_signed_int_tag;
    meas.value.signed_int = value;

    return EncodeSensorMeasurement(&meas, buffer, size);
}

SensorStatus EncodeDoubleMeasurement(Metadata meta, double value, SensorType type,
                             uint8_t* buffer, size_t* size) {
    SensorMeasurement meas = SensorMeasurement_init_zero;

    meas.meta = meta;
    meas.has_meta = true;

    meas.type = type;
    meas.which_value = SensorMeasurement_decimal_tag;

    return EncodeSensorMeasurement(&meas, buffer, size);
}


SensorStatus DecodeSensorMeasurement(const uint8_t* data, const size_t len,
                             SensorMeasurement* meas) {
    pb_istream_t istream = pb_istream_from_buffer(data, len);
    bool status = pb_decode(&istream, SensorMeasurement_fields, meas);
    if (!status) {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}


SensorStatus DecodeRepeatedSensorMeasurements(const uint8_t* data, const size_t len,
                                     RepeatedSensorMeasurements* measurements) {
    pb_istream_t istream = pb_istream_from_buffer(data, len);
    bool status = pb_decode(&istream, RepeatedSensorMeasurements_fields, measurements);
    if (!status) {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}
