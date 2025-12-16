"""Tests encoding/decoding of sensor data."""

import unittest

from ents.proto.sensor import (
    encode_sensor_measurement,
    encode_repeated_sensor_measurements,
    decode_sensor_measurement,
    decode_repeated_sensor_measurements,
    update_repeated_metadata,
)

class TestProtoSensor(unittest.TestCase):
    """Tests encoding/decoding of sensor data."""

    def test_sensor_measurement(self):
        """Tests encoding/decoding of single SensorMeasurement."""

        # encode

        meas_in = {
            "metadata": {
                "ts": 123,
                "logger_id": 456,
                "cell_id": 789,
            },
            "type": POWER_VOLTAGE,
            "unsigned_int": 100,
        }

        serialized = encode_sensor_measurement(meas_in)

        # decode
        meas_out = decode_sensor_measurement(serialized)

        self.assertEqual(meas_in, meas_out)


    def test_repeated_sensor_measurements(self):
        """Tests encoding/decoding of RepeatedSensorMeasurement."""

        meas_in = {
            "metadata": {
                "ts": 123,
                "logger_id": 456,
                "cell_id": 789,
            },

            "measurements": [
                {
                    "metadata": {
                        "ts": 124,
                        "logger_id": 457,
                        "cell_id": 790,
                    },
                    "type": POWER_VOLTAGE,
                    "unsigned_int": 100,
                },
                {
                    "metadata": {
                        "ts": 125,
                        "logger_id": 458,
                        "cell_id": 791,
                    },
                    "type": POWER_CURRENT,
                    "signed_int": -100,
                },
            ]
        }

        serialized = encode_repeated_sensor_measurements(meas_in)

        meas_out = decode_repeated_sensor_measurements(serialized)

        self.assertEqual(meas_in, meas_out)

    def test_update_repeated_metadata_no_change(self):
        """Tests when no change should occur in the measurement."""
        
        meas_in = {
            "metadata": {
                "ts": 123,
                "logger_id": 456,
                "cell_id": 789,
            },

            "measurements": [
                {
                    "metadata": {
                        "ts": 124,
                        "logger_id": 457,
                        "cell_id": 790,
                    },
                    "type": POWER_VOLTAGE,
                    "unsigned_int": 100,
                },
                {
                    "metadata": {
                        "ts": 125,
                        "logger_id": 458,
                        "cell_id": 791,
                    },
                    "type": POWER_CURRENT,
                    "signed_int": -100,
                },
            ]
        }

        meas_out = update_repeated_metadata(meas_in)

        self.assertEqual(meas_in, meas_out)

    def test_update_repeated_metadata_ok(self):
        """Tests the typical case of updating repeated measurement metadata."""
        
        meas_in = {
            "metadata": {
                "ts": 123,
                "logger_id": 456,
                "cell_id": 789,
            },

            "measurements": [
                {
                    "type": POWER_VOLTAGE,
                    "unsigned_int": 100,
                },
                {
                    "metadata": {
                        "ts": 125,
                        "logger_id": 458,
                        "cell_id": 791,
                    },
                    "type": POWER_CURRENT,
                    "signed_int": -100,
                },
            ]
        }

        meas_out = update_repeated_metadata(meas_in)

        self.assertIn("metadata", meas_out["measurements"][0])
        self.assertEqual(meas_out["measurements"][0]["metadata"],
                         meas_in["metadata"])

def test_update_repeated_metadata_value_error(self):
        """Tests ValueError when repeated is missing metadata."""

        meas_in = {
            "measurements": [
                {
                    "metadata": {
                        "ts": 124,
                        "logger_id": 457,
                        "cell_id": 790,
                    },
                    "type": POWER_VOLTAGE,
                    "unsigned_int": 100,
                },
                {
                    "metadata": {
                        "ts": 125,
                        "logger_id": 458,
                        "cell_id": 791,
                    },
                    "type": POWER_CURRENT,
                    "signed_int": -100,
                },
            ]
        }

        with self.assertRaises(ValueError):
            meas_out = update_repeated_metadata(meas_in)

if __name__ == "__main__":
    unittest.main()
