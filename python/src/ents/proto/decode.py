"""Module to decode soil power sensor messages"""

from google.protobuf.json_format import MessageToDict

from .soil_power_sensor_pb2 import (
    Measurement,
    RepeatedMeasurement,
    Response,
    UserConfiguration,
)


def decode_response(data: bytes):
    """Decodes a Response message.

    Args:
        data: Byte array of Response message.

    Returns:
        Returns the ResponseType.

    Raises:
        KeyError: Missing the resp field.
    """

    resp = Response()
    resp.ParseFromString(data)

    if not resp.HasField("resp"):
        raise KeyError("Missing response type")

    return resp.resp


def decode_repeated_measurement(data: bytes) -> list[dict]:
    """Decodes repeated measurements

    Args:
        data: Byte array from RepeatedMeasurement

    Returns:
        List of decoded measurement dictionariess.
    """

    rep_meas = RepeatedMeasurement()
    rep_meas.ParseFromString(data)

    parsed_meas_list : list[dict] = []

    for meas in rep_meas.measurements:
        # set meta from repeated measurement if available
        if not meas.HasField("meta"):
            meas.meta = rep_meas.meta

        parsed_meas = parse_measurement(meas)

        parsed_meas_list.append(parsed_meas)

    return parsed_meas_list


def decode_measurement(data: bytes, raw: bool = True) -> dict:
    """Decodes a Measurement message

    The data is decoded into a flat dictionary with the measurement type.

    Args:
        data: Byte array of Measurement message.
        raw: Flag to return raw or adjusted measurements (deprecated). Was used
        to adjust individual measurements which is now left up to the user.

    Returns:
        Flat dictionary of values from the meta field, measurement field, and
        the key "type" to indicate the type of measurement.
    """

    del raw

    # parse data
    meas = Measurement()
    meas.ParseFromString(data)

    meas_dict = parse_measurement(meas)

    return meas_dict


def parse_measurement(meas : Measurement) -> dict:
    """Parses a measurement object into a dictionary

    {
        "ts": 1345531
        "logger_id": 200
        "cell_id": 200
        "type": "bme280"
        "data":
            "pressure": 124
            "temperature": 21.99
            "humidity": 40.0
        "data_type":
            "pressure": int
            "temperature": float
            "humidity": float
    }

    Args:
        meas: Measurement object

    Returns:
        Dictionary representation of the measurement.

    Raises:
        KeyError: When the serialized data is missing a required field.
    """

    # convert meta into dict
    if not meas.HasField("meta"):
        raise KeyError("Measurement missing metadata")
    meta_dict = MessageToDict(meas.meta)

    # decode measurement
    if not meas.HasField("measurement"):
        raise KeyError("Measurement missing data")
    measurement_type = meas.WhichOneof("measurement")
    measurement_dict = MessageToDict(getattr(meas, measurement_type))

    # store measurement type
    meta_dict["type"] = measurement_type

    # store measurement data
    meta_dict["data"] = measurement_dict

    # store measurement type
    meta_dict["data_type"] = {}
    for key, value in measurement_dict.items():
        meta_dict["data_type"][key] = type(value)


def decode_user_configuration(data: bytes) -> dict:
    """Decodes a UserConfiguration message

    Args:
        data: Byte array of UserConfiguration message.

    Returns:
        Dictionary of UserConfiguration values.

    Raises:
        KeyError: When the serialized data is missing a required field.
    """

    user_config = UserConfiguration()
    user_config.ParseFromString(data)

    if user_config.cell_id == 0 or user_config.logger_id == 0:
        raise KeyError("User configuration missing required fields")

    user_config_dict = MessageToDict(
        user_config, always_print_fields_with_no_presence=True
    )

    return user_config_dict
