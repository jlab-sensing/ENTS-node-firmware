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


SensorStatus FormatRepeatedSensorMeasurements(
        Metadata meta, const SensorMeasurement meas[], size_t count,
        RepeatedSensorMeasurements* out) {
    
    // check count doesn't exceed max count
    size_t max_count = sizeof(out->measurements) / sizeof(out->measurements[0]);
    if (count > max_count) {
        return SENSOR_OUT_OF_BOUNDS;
    }

    //*out = RepeatedSensorMeasurements_init_zero;

    // handle metadata
    if (MetadataEqual(&meta, &METADATA_NONE)) {
        out->has_meta = false;
    } else {
        out->has_meta = true;
        out->meta = meta;
    }

    // copy measurements
    out->measurements_count = count;
    for (size_t i = 0; i < count; i++) {
        out->measurements[i] = meas[i];
    }

    return SENSOR_OK;
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

    // format into repeated structure
    SensorStatus sensor_status = FormatRepeatedSensorMeasurements(
        meta, meas, count, &rep_meas);
    if (sensor_status != SENSOR_OK) {
        return sensor_status;
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


SensorStatus RepeatedSensorMeasurementsSize(Metadata meta, const SensorMeasurement meas[], 
        size_t count, size_t *size) {

    RepeatedSensorMeasurements rep_meas = RepeatedSensorMeasurements_init_zero;

    // format into repeated structure
    SensorStatus sensor_status = FormatRepeatedSensorMeasurements(
        meta, meas, count, &rep_meas);
    if (sensor_status != SENSOR_OK) {
        return sensor_status;
    }

    // get size
    bool status = pb_get_encoded_size(
            size,
            RepeatedSensorMeasurements_fields,
            &rep_meas);
    if (!status) {
        return SENSOR_ERROR;
    }

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





SensorStatus EncodeRepeatedSensorResponses(const RepeatedSensorResponses responses,
    size_t count, uint8_t* buffer, size_t* size) {

    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, *size);
    bool status = pb_encode(&ostream, RepeatedSensorResponses_fields, &responses);
    if (!status) {
        return SENSOR_ERROR;
    }

    *size = ostream.bytes_written;
    return SENSOR_OK;
}




SensorStatus DecodeRepeatedSensorReponses(const uint8_t* data, const size_t len,
                             RepeatedSensorResponses* resp) {
    pb_istream_t istream = pb_istream_from_buffer(data, len);
    bool status = pb_decode(&istream, RepeatedSensorResponses_fields, resp);
    if (!status) {
        return SENSOR_ERROR;
    }

    return SENSOR_OK;
}

SensorStatus CheckSensorResponse(const SensorResponse* resp) {
    // Handle an overall error for repeated sensor measurement 
    if (resp->idx == 0) {
        switch (resp->error) {
            case SensorError_OK:
                return SENSOR_OK;
            default:
                return SENSOR_REUPLOAD;
        }
    // Otherwise handle on individual measurement
    } else {
        switch (resp->error) {
            case SensorError_OK:
                return SENSOR_OK;
            case SensorError_LOGGER:
            case SensorError_CELL:
            case SensorError_UNSUPPORTED:
                return SENSOR_FORMAT;
            default:
                return SENSOR_REUPLOAD;
        }
    }

    return SENSOR_OK;
}
