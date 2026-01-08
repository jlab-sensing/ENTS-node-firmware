#include "sensor.h"

#include "pb_decode.h"
#include "pb_encode.h"


/**
 * @brief Compares two Metadata structures for equality.
 *
 * @param left Pointer to the first Metadata structure.
 * @param right Pointer to the second Metadata structure.
 *
 * @return true if equal, false otherwise.
 */
bool MetadataEqual(Metadata* left, Metadata* right);

/**
 * @brief Optimizes repeated sensor measurements for size.
 *
 * Places duplicate metadata fields into the top level "meta" field.
 *
 * @param meas Pointer to the RepeatedSensorMeasurements to optimize.
 *
 * @returns The number of duplicate metadata removed.
 */
unsigned int Format(RepeatedSensorMeasurements* meas);



/**
 * @brief Parses repeated sensor measurements to add metadata back to
 * individual measurements.
 *
 * @param meas Pointer to the RepeatedSensorMeasurements to parse.
 *
 * @return SENSOR_OK on success, SENSOR_ERROR on failure.
 */
SensorStatus Parse(RepeatedSensorMeasurements* meas);


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


unsigned int Format(RepeatedSensorMeasurements* meas) {    
    unsigned int most_count = 1;

    // find the most repeated metadata 
    for (int first = 0; first < meas->measurements_count; first++) {
        unsigned int current_count = 1;
        for (int second = 0; second < meas->measurements_count; second++) {
           

            // skip over yourself
            if (first == second) {
                continue;
            }

            // check for duplicates
            Metadata* first_meta = &meas->measurements[first].meta;
            Metadata* second_meta = &meas->measurements[second].meta;
            if (MetadataEqual(first_meta, second_meta)) {
                current_count++;
            }
        }

        // update if there are more duplicates
        if (current_count > most_count) {
            most_count = current_count;
            meas->meta = meas->measurements[first].meta;
        }
    }

    // return early if there is no duplicates
    if (most_count < 2) {
        return 1;
    }


    // enable top level metadata and delete duplicates
    meas->has_meta = true;
    for (int i = 0; i < meas->measurements_count; i++) {
        Metadata* current_meta = &meas->measurements[i].meta;
        if (MetadataEqual(&meas->meta, current_meta)) {
            meas->measurements[i].has_meta = false;
        }
    }

    return most_count;
}



SensorStatus Parse(RepeatedSensorMeasurements* meas) {
    // if no top level metadata, check each measurements has metadata
    if (!meas->has_meta) {
        for (size_t i = 0; i < meas->measurements_count; i++) {
            if (!meas->measurements[i].has_meta) {
                return SENSOR_ERROR;
            }
        }
    } else {
        // add top level metadata to measurements missing it
        for (size_t i = 0; i < meas->measurements_count; i++) {
            if (!meas->measurements[i].has_meta) {
                meas->measurements[i].meta = meas->meta;
                meas->measurements[i].has_meta = true;
            }
        }
    }

    return SENSOR_OK;
}


SensorStatus FormatRepeatedSensorMeasurements(
        Metadata meta, const SensorMeasurement meas[], size_t count,
        RepeatedSensorMeasurements* out) {
    
    // check count doesn't exceed max count
    size_t max_count = sizeof(out->measurements) / sizeof(out->measurements[0]);
    if (count > max_count) {
        return SENSOR_OUT_OF_BOUNDS;
    }

    // cannot reset a pointer directly, just hoping this works
    //*out = RepeatedSensorMeasurements_init_zero;

    // copy measurements
    out->measurements_count = count;
    for (size_t i = 0; i < count; i++) {
        out->measurements[i] = meas[i];
    }

    Format(out);

    return SENSOR_OK;
}


SensorStatus EncodeSensorMeasurement(const SensorMeasurement *meas, uint8_t *buffer, size_t *size) {
    // create output stream
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, 256);
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
    size_t count, uint8_t* buffer, size_t size, size_t* length) {

    RepeatedSensorMeasurements rep_meas = RepeatedSensorMeasurements_init_zero;

    // format into repeated structure
    SensorStatus sensor_status = FormatRepeatedSensorMeasurements(
        meta, meas, count, &rep_meas);
    if (sensor_status != SENSOR_OK) {
        return sensor_status;
    }

    // encode
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, size);
    bool status = pb_encode(&ostream, RepeatedSensorMeasurements_fields, &rep_meas);
    if (!status) {
        return SENSOR_ERROR;
    }

    *length = ostream.bytes_written;
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


SensorStatus EncodeUint32Measurement(Metadata meta,  uint32_t value, SensorType type,
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

SensorStatus EncodeInt32Measurement(Metadata meta, int32_t value, SensorType type,
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
    meas.value.decimal = value;

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

    return Parse(measurements);
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
