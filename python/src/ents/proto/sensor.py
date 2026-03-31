"""Module for sensor measurements

Encode/decoding functions are wrappers around the protobuf messages. These take
in the json dictionary format of the messages and return serialized byte
arrays.

Format/parse functions implement the protocol for repeated sensor measurements.
These take in a list of measurements and automatically optimize repeated
metadata fields.
"""

from google.protobuf.json_format import MessageToDict, ParseDict

from .sensor_pb2 import (
    SensorMeasurement,
    RepeatedSensorMeasurements,
    SensorType,
    RepeatedSensorResponses,
)


def parse_sensor_measurement(data: bytes) -> list:
    """Parses a sensor measurement into a usable dictionary.

    Function does the following:
        1. Decodes the serialized byte array
        2. Updates metadata for each measurement if missing
        3. Adds names, descriptions, and units to metadata

    Args:
        data: Byte array of message.

    Returns:
        Dictionary of sensor measurement.
    """

    meas = decode_repeated_sensor_measurements(data)
    meas = update_repeated_metadata(meas)
    for m in meas["measurements"]:
        sensor_data = get_sensor_data(m["type"])
        m.update(sensor_data)

    return meas


def format_sensor_measurement(meas: list) -> bytes:
    """Formats a sensor measurement dictionary into a serialized byte array.

    Function does the following:
        1. Uses top level metadata for duplicate measurement metadata fields
        2. Encodes the dictionary into a serialized byte array

    Args:
        meas: Dictionary of sensor measurement.

    Returns:
        Byte array of serialized message.
    """

    # TODO Implement optimization of repeated metadata fields
    meas_dict = {
        "measurements": meas,
    }

    data = encode_repeated_sensor_measurements(meas_dict)
    return data


def get_sensor_data(meas_type: int) -> dict:
    """Gets sensor data information.

    Args:
        meas_type: Sensor measurement dictionary.

    Returns:
        Metadata associated with the sensor type.
    """

    SENSOR_DATA = {
        SensorType.POWER_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        SensorType.POWER_CURRENT: {
            "name": "Current",
            "unit": "uA",
        },
        SensorType.TEROS12_VWC: {
            "name": "Volumetric Water Content (Raw)",
            "unit": "raw",
        },
        SensorType.TEROS12_VWC_ADJ: {
            "name": "Volumetric Water Content",
            "unit": "%",
        },
        SensorType.TEROS12_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },
        SensorType.TEROS12_EC: {
            "name": "Electrical Conductivity",
            "unit": "uS/cm",
        },
        SensorType.PHYTOS31_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        SensorType.PHYTOS31_LEAF_WETNESS: {
            "name": "Leaf Wetness",
            "unit": "%",
        },
        SensorType.BME280_PRESSURE: {
            "name": "Pressure",
            "unit": "kPa",
        },
        SensorType.BME280_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },
        SensorType.BME280_HUMIDITY: {
            "name": "Humidity",
            "unit": "%",
        },
        SensorType.TEROS21_MATRIC_POT: {
            "name": "Matric Potential",
            "unit": "kPa",
        },
        SensorType.TEROS21_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },
        SensorType.SEN0308_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        SensorType.SEN0308_HUMIDITY: {
            "name": "Humidity",
            "unit": "%",
        },
        SensorType.SEN0257_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        SensorType.SEN0257_PRESSURE: {
            "name": "Pressure",
            "unit": "kPa",
        },
        SensorType.YFS210C_FLOW: {
            "name": "Flow Rate",
            "unit": "L/min",
        },
        SensorType.PCAP02_CAPACITANCE: {
            "name": "Capacitance",
            "unit": "pF",
        },
    }

    meta = SENSOR_DATA[SensorType.Value(meas_type)]
    return meta


def encode_sensor_measurement(meas_dict: dict) -> bytes:
    meas = SensorMeasurement()
    ParseDict(meas_dict, meas)

    return meas.SerializeToString()


def encode_repeated_sensor_measurements(meas_dict: dict) -> bytes:
    """Encodes a SensorMeasurement message

    Args:
        rep_meas: Repeated sensor measurement dictionary.

    Returns:
        Byte array of encoded RepeatedSensorMeasurements message.
    """

    meas = RepeatedSensorMeasurements()
    ParseDict(meas_dict, meas)

    return meas.SerializeToString()


def decode_sensor_measurement(data: bytes) -> dict:
    """Decodes a SensorMeasurement message

    Args:
        data: Byte array of SensorMeasurement message.

    Returns:
        Decoded sensor measurement dictionary.
    """

    meas = SensorMeasurement()
    meas.ParseFromString(data)

    parsed_meas = MessageToDict(meas)

    return parsed_meas


def decode_repeated_sensor_measurements(data: bytes) -> dict:
    """Decodes repeated sensor measurements

    Args:
        data: Byte array from RepeatedSensorMeasurements

    Returns:
        List of decoded sensor measurement dictionaries.
    """

    rep_meas = RepeatedSensorMeasurements()
    rep_meas.ParseFromString(data)
    return MessageToDict(rep_meas)


def update_repeated_metadata(meas: dict) -> dict:
    """Ensures every measurements has metadata field set.

    If a measurement is missing the metadata field, it is filled in from the
    repeated sensor measurement. Existing measurement metadata fields are not
    overwritten.

    Args:
        meas: Sensor measurement dictionary.

    Returns:
        Updated sensor measurement dictionary.
    """

    # if top level meta does not exist, ensure all measurements have meta
    if "meta" not in meas:
        for m in meas["measurements"]:
            if "meta" not in m:
                raise ValueError("Repeated measurement missing metadata field.")
    # otherwise populate missing measurement meta from top level
    else:
        for m in meas["measurements"]:
            if "meta" not in m:
                m["meta"] = meas["meta"]
        del meas["meta"]

    return meas


def encode_sensor_response(resp_dict: dict) -> bytes:
    """Encodes a sensor response message.

    {
        responses: [
            {
                status: int,
                message: str,
            },
            ...
        ]
    }

    Args:
        resp_dict: Sensor response dictionary.

    Returns:
        Byte array of encoded SensorResponse message.
    """

    resp = RepeatedSensorResponses()
    ParseDict(resp_dict, resp)

    return resp.SerializeToString()


def decode_sensor_response(data: bytes) -> dict:
    """Decodes a sensor response message.

    {
        responses: [
            {
                status: int,
                message: str,
            },
            ...
        ]
    }

    Args:
        data: Byte array of SensorResponse message.

    Returns:
        Decoded sensor response dictionary.
    """

    resp = RepeatedSensorResponses()
    resp.ParseFromString(data)

    parsed_resp = MessageToDict(resp)

    return parsed_resp
