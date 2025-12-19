"""Tests encoding/decoding of sensor data."""

import unittest

from ents.proto.sensor import (
    encode_sensor_measurement,
    encode_repeated_sensor_measurements,
    decode_sensor_measurement,
    decode_repeated_sensor_measurements,
    update_repeated_metadata,
    get_sensor_data,
)

class TestProtoSensor(unittest.TestCase):
    """Tests encoding/decoding of sensor data."""

    def test_sensor_measurement(self):
        """Tests encoding/decoding of single SensorMeasurement."""

        # encode

        meas_in = {
            "meta": {
                "ts": 123,
                "loggerId": 456,
                "cellId": 789,
            },
            "type": "POWER_VOLTAGE",
            "unsignedInt": 100,
        }

        serialized = encode_sensor_measurement(meas_in)

        # decode
        meas_out = decode_sensor_measurement(serialized)

        self.assertEqual(meas_in, meas_out)


    def test_repeated_sensor_measurements(self):
        """Tests encoding/decoding of RepeatedSensorMeasurement."""

        meas_in = {
            "meta": {
                "ts": 123,
                "loggerId": 456,
                "cellId": 789,
            },

            "measurements": [
                {
                    "meta": {
                        "ts": 124,
                        "loggerId": 457,
                        "cellId": 790,
                    },
                    "type": "POWER_VOLTAGE",
                    "unsignedInt": 100,
                },
                {
                    "meta": {
                        "ts": 125,
                        "loggerId": 458,
                        "cellId": 791,
                    },
                    "type": "POWER_CURRENT",
                    "signedInt": -100,
                },
            ]
        }
        
        meas_expected = [
                {
                    "meta": {
                        "ts": 124,
                        "loggerId": 457,
                        "cellId": 790,
                    },
                    "type": "POWER_VOLTAGE",
                    "unsignedInt": 100,
                },
                {
                    "meta": {
                        "ts": 125,
                        "loggerId": 458,
                        "cellId": 791,
                    },
                    "type": "POWER_CURRENT",
                    "signedInt": -100,
                },
            ]

        serialized = encode_repeated_sensor_measurements(meas_in)

        meas_out = decode_repeated_sensor_measurements(serialized)

        self.assertEqual(meas_expected, meas_out)

    def test_update_repeated_metadata_no_change(self):
        """Tests when no change should occur in the measurement."""

        meas_in = {
            "meta": {
                "ts": 123,
                "loggerId": 456,
                "cellId": 789,
            },

            "measurements": [
                {
                    "meta": {
                        "ts": 124,
                        "loggerId": 457,
                        "cellId": 790,
                    },
                    "type": "POWER_VOLTAGE",
                    "unsignedInt": 100,
                },
                {
                    "meta": {
                        "ts": 125,
                        "loggerId": 458,
                        "cellId": 791,
                    },
                    "type": "POWER_CURRENT",
                    "signedInt": -100,
                },
            ]
        }

        meas_out = update_repeated_metadata(meas_in)

        self.assertEqual(meas_in, meas_out)

    def test_update_repeated_metadata_ok(self):
        """Tests the typical case of updating repeated measurement metadata."""
        
        meas_in = {
            "meta": {
                "ts": 123,
                "loggerId": 456,
                "cellId": 789,
            },

            "measurements": [
                {
                    "type": "POWER_VOLTAGE",
                    "unsignedInt": 100,
                },
                {
                    "meta": {
                        "ts": 125,
                        "loggerId": 458,
                        "cellId": 791,
                    },
                    "type": "POWER_CURRENT",
                    "signedInt": -100,
                },
            ]
        }

        meas_out = update_repeated_metadata(meas_in)

        self.assertIn("meta", meas_out["measurements"][0])
        self.assertEqual(meas_out["measurements"][0]["meta"],
                         meas_in["meta"])

    def test_update_repeated_metadata_value_error(self):
            """Tests ValueError when repeated is missing metadata."""

            meas_in = {
                "measurements": [
                    {
                        "meta": {
                            "ts": 124,
                            "loggerId": 457,
                            "cellId": 790,
                        },
                        "type": "POWER_VOLTAGE",
                        "unsignedInt": 100,
                    },
                    {
                        "meta": {
                            "ts": 125,
                            "loggerId": 458,
                            "cellId": 791,
                        },
                        "type": "POWER_CURRENT",
                        "signedInt": -100,
                    },
                ]
            }

            with self.assertRaises(ValueError):
                _ = update_repeated_metadata(meas_in)

    def test_get_sensor_data(self):
        """Tests get_sensor_data function."""

        meas_type = "POWER_VOLTAGE"

        sensor_data = get_sensor_data(meas_type)

        expected_data = {
            "name": "Voltage",
            "unit": "mV",
        }

        self.assertEqual(sensor_data, expected_data)

if __name__ == "__main__":
    unittest.main()
