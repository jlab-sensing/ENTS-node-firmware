"""Module for sensor measurements"""


from google.protobuf.json_format import MessageToDict, ParseDict

from .sensor_pb2 import *

def parse_sensor_measurement(data: bytes) -> dict:
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
    meas = add_sensor_data(meas)

    return meas


def add_sensor_data(meas: dict) -> dict:
    """Adds names, descriptions, and units to sensor measurement metadata.

    Args:
        meas: Sensor measurement dictionary.

    Returns:
        Updated sensor measurement dictionary.
    """

    SENSOR_DATA = {
        POWER_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        POWER_CURRENT: {
            "name": "Current",
            "unit": "uA",
        },

        TEROS12_VWC: {
            "name": "Volumetric Water Content",
            "unit": "%",
        },
        TEROS12_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },
        TEROS12_EC: {
            "name": "Electrical Conductivity",
            "unit": "uS/cm",
        },

        PHYTOS31_VOLTAGE: {
            "name": "Voltage",
            "unit": "mV",
        },
        PHYTOS31_LEAF_WETNESS: {
            "name": "Leaf Wetness",
            "unit": "%",
        },

        BME280_PRESSURE: {
            "name": "Pressure",
            "unit": "kPa",
        },
        BMEE280_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },
        BME280_HUMIDITY: {
            "name": "Humidity",
            "unit": "%",
        },

        TEROS21_MATRIC_POT: {
            "name": "Matric Potential",
            "unit": "kPa",
        },
        TEROS21_TEMP: {
            "name": "Temperature",
            "unit": "C",
        },

        SEN0308_VOLTAGE: {

        }

    }

    return meas


def encode_sensor_measurement(meas_dict: dict) -> bytes:
    meas = SensorMeasurement()
    ParseDict(meas_dict, meas)

    return meas.SerializeToString()

def encode_repeated_sensor_measurements(meas_dict: dict) -> bytes:
    """Encodes a SensorMeasurement message

    Args:
        rep_meas: Repeated sensor measurement dictionary.

    Returns:
        Byte array of encoded RepeatedSensorMeasurement message.
    """

    meas = RepeatedSensorMeasurement()
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


def decode_repeated_sensor_measurements(data: bytes) -> list[dict]:
    """Decodes repeated sensor measurements

    Args:
        data: Byte array from RepeatedSensorMeasurement

    Returns:
        List of decoded sensor measurement dictionaries.
    """

    rep_meas = RepeatedSensorMeasurement()
    rep_meas.ParseFromString(data)

    parsed_meas_list : list[dict] = []

    for meas in rep_meas.measurements:
        # set meta from repeated measurement if available
        if not meas.HasField("meta"):
            meas.meta = rep_meas.meta

        parsed_meas = MessageToDict(meas)

        parsed_meas_list.append(parsed_meas)

    return parsed_meas_list


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

    if "metadata" not in meas:
        raise ValueError("Repeated measurement missing metadata field.")

    for m in meas["measurements"]:
        if "metadata" not in m:
            m["metadata"] = meas["metadata"]

    return meas
