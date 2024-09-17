"""Module to decode soil power sensor messages
"""

from google.protobuf.json_format import MessageToDict

from .soil_power_sensor_pb2 import Measurement, Response


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


def decode_measurement(data: bytes) -> dict:
    """Decodes a Measurement message

    The data is decoded into a flat dictionary with the measurement type.

    Args:
        data: Byte array of Measurement message.

    Returns:
        Flat dictionary of values from the meta field, measurement field, and
        the key "type" to indicate the type of measurement.

    Raises:
        KeyError: When the serialized data is missing a required field.
    """

    # parse data
    meas = Measurement()
    meas.ParseFromString(data)

    # convert meta into dict
    if not meas.HasField("meta"):
        raise KeyError("Measurement missing metadata")
    meta_dict = MessageToDict(meas.meta)

    # decode measurement
    if not meas.HasField("measurement"):
        raise KeyError("Measurement missing data")
    measurement_type = meas.WhichOneof("measurement")
    measurement_dict = MessageToDict(
        getattr(meas, measurement_type)
    )

    # store measurement type
    meta_dict["type"] = measurement_type

    # store measurement data
    meta_dict["data"] = measurement_dict

    # store measurement type
    meta_dict["data_type"] = {}
    for key, value in measurement_dict.items():
        meta_dict["data_type"][key] = type(value)

    return meta_dict
