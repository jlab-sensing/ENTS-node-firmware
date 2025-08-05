"""Tests encoding and decoding of protobuf serialized data

Encoding for Response messages checks both a SUCCESS and ERROR can be obtained.
Decoding is performed to ensure data is preserved.

Decoding checks that the Measurement message is preserved through an
encoding/decoding cycle. Checks that missing fields result in an error and the
correct dictionary format is returned.
"""

import unittest
import base64

from ents.proto import (
    encode_response,
    decode_measurement,
    encode_esp32command,
    decode_esp32command,
)

from ents.proto.soil_power_sensor_pb2 import (
    Measurement,
    Response,
    MeasurementMetadata,
    Esp32Command,
    PageCommand,
    TestCommand,
)


class TestEncode(unittest.TestCase):
    def test_success(self):
        # encode
        resp_str = encode_response(success=True)

        # decode
        resp_out = Response()
        resp_out.ParseFromString(resp_str)

        self.assertEqual(Response.ResponseType.SUCCESS, resp_out.resp)

    def test_error(self):
        # encode
        resp_str = encode_response(success=False)

        # decode
        resp_out = Response()
        resp_out.ParseFromString(resp_str)

        self.assertEqual(Response.ResponseType.ERROR, resp_out.resp)


class TestDecode(unittest.TestCase):
    """Test decoding of measurements"""

    def setUp(self):
        """Creates a default metadata message"""
        self.meta = MeasurementMetadata()
        self.meta.ts = 1436079600
        self.meta.cell_id = 20
        self.meta.logger_id = 4

    def check_meta(self, meas_dict: dict):
        """Checks the measurement dictionary contains metadata information"""

        self.assertEqual(1436079600, meas_dict["ts"])
        self.assertEqual(20, meas_dict["cellId"])
        self.assertEqual(4, meas_dict["loggerId"])

    def test_power(self):
        """Test decoding of PowerMeasurement"""

        # import pdb; pdb.set_trace()
        # format measurement
        meas = Measurement()
        meas.meta.CopyFrom(self.meta)
        meas.power.voltage = 122.38
        meas.power.current = 514.81

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        meas_dict = decode_measurement(data=meas_str)

        # check resulting dict
        self.assertEqual("power", meas_dict["type"])
        self.check_meta(meas_dict)
        self.assertAlmostEqual(122.38, meas_dict["data"]["voltage"])
        self.assertEqual(float, meas_dict["data_type"]["voltage"])
        self.assertAlmostEqual(514.81, meas_dict["data"]["current"])
        self.assertEqual(float, meas_dict["data_type"]["current"])

    def test_teros12(self):
        """Test decoding of Teros12Measurement"""

        # format measurement
        meas = Measurement()
        meas.meta.CopyFrom(self.meta)
        meas.teros12.vwc_raw = 2124.62
        meas.teros12.vwc_adj = 0.43
        meas.teros12.temp = 24.8
        meas.teros12.ec = 123

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        meas_dict = decode_measurement(data=meas_str)

        # check dict
        self.assertEqual("teros12", meas_dict["type"])
        self.check_meta(meas_dict)
        self.assertAlmostEqual(2124.62, meas_dict["data"]["vwcRaw"])
        self.assertEqual(float, meas_dict["data_type"]["vwcRaw"])
        self.assertAlmostEqual(0.43, meas_dict["data"]["vwcAdj"])
        self.assertEqual(float, meas_dict["data_type"]["vwcAdj"])
        self.assertAlmostEqual(24.8, meas_dict["data"]["temp"])
        self.assertEqual(float, meas_dict["data_type"]["temp"])
        self.assertEqual(123, meas_dict["data"]["ec"])
        self.assertEqual(int, meas_dict["data_type"]["ec"])

    def test_bme280(self):
        """Test decoding of bme280 measurement"""

        meas = Measurement()
        meas.meta.CopyFrom(self.meta)
        meas.bme280.pressure = 98473
        meas.bme280.temperature = 2275
        meas.bme280.humidity = 43600

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        meas_dict = decode_measurement(data=meas_str)

        # check dict
        self.assertEqual("bme280", meas_dict["type"])
        self.check_meta(meas_dict)
        self.assertEqual(98473, meas_dict["data"]["pressure"])
        self.assertEqual(int, meas_dict["data_type"]["pressure"])
        self.assertEqual(2275, meas_dict["data"]["temperature"])
        self.assertEqual(int, meas_dict["data_type"]["temperature"])
        self.assertEqual(43600, meas_dict["data"]["humidity"])
        self.assertEqual(int, meas_dict["data_type"]["humidity"])

        # decode
        meas_dict = decode_measurement(data=meas_str, raw=False)

        # check dict
        self.assertEqual("bme280", meas_dict["type"])
        self.check_meta(meas_dict)
        self.assertAlmostEqual(9847.3, meas_dict["data"]["pressure"])
        self.assertEqual(float, meas_dict["data_type"]["pressure"])
        self.assertAlmostEqual(22.75, meas_dict["data"]["temperature"])
        self.assertEqual(float, meas_dict["data_type"]["temperature"])
        self.assertAlmostEqual(43.600, meas_dict["data"]["humidity"])
        self.assertEqual(float, meas_dict["data_type"]["humidity"])

    def test_teros21(self):
        meas = Measurement()
        meas.meta.CopyFrom(self.meta)

        meas.teros21.matric_pot = 101.23
        meas.teros21.temp = 22.50

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        meas_dict = decode_measurement(data=meas_str)

        # check dict
        self.assertEqual("teros21", meas_dict["type"])
        self.check_meta(meas_dict)
        self.assertAlmostEqual(101.23, meas_dict["data"]["matricPot"])
        self.assertEqual(float, meas_dict["data_type"]["matricPot"])
        self.assertAlmostEqual(22.50, meas_dict["data"]["temp"])
        self.assertEqual(float, meas_dict["data_type"]["temp"])

    def test_missing_meta(self):
        """Test that error is raised when meta is not set"""

        # format measurement
        meas = Measurement()
        meas.power.voltage = 122.38
        meas.power.current = 514.81

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        with self.assertRaises(KeyError):
            decode_measurement(data=meas_str)

    def test_missing_measurement(self):
        """Test that error is raised when measurement is missing"""

        # format measurement
        meas = Measurement()
        meas.meta.CopyFrom(self.meta)

        # serialize
        meas_str = meas.SerializeToString()

        # decode
        with self.assertRaises(KeyError):
            decode_measurement(data=meas_str)

    def test_missing_default_values(self):
        """Test to ensure that default valued fields are included in the
        decoded dictionary
        """

        meas = Measurement()
        meas.meta.CopyFrom(self.meta)

        meas.teros12.vwc_raw = 2141.52
        meas.teros12.vwc_adj = 0.45
        meas.teros12.temp = 25.0

        # set integer to default value of 0
        meas.teros12.ec = 0

        meas_str = meas.SerializeToString()

        meas_dict = decode_measurement(data=meas_str)

        self.assertIn("ec", meas_dict["data"])


class TestEsp32(unittest.TestCase):
    def test_cmd_not_implemented(self):
        """Checks that an exception is raised when a non-existing command is
        called"""

        with self.assertRaises(NotImplementedError):
            encode_esp32command("agg", req="open", fd=123, bs=456, n=789)

    def test_page_encode(self):
        """Test encoding a page command"""

        req = "open"
        fd = 1
        bs = 512
        n = 1024

        cmd_str = encode_esp32command("page", req=req, fd=fd, bs=bs, n=n)

        cmd = Esp32Command()
        cmd.ParseFromString(cmd_str)

        # check the command type
        cmd_type = cmd.WhichOneof("command")
        self.assertEqual(cmd_type, "page_command")

        # check individual values
        self.assertEqual(cmd.page_command.file_request, PageCommand.RequestType.OPEN)
        self.assertEqual(cmd.page_command.file_descriptor, fd)
        self.assertEqual(cmd.page_command.block_size, bs)
        self.assertEqual(cmd.page_command.num_bytes, n)

    def test_page_decode(self):
        """Test decoding a page command"""

        req = "open"
        fd = 1
        bs = 512
        n = 1024

        cmd_str = encode_esp32command("page", req=req, fd=fd, bs=bs, n=n)

        cmd = decode_esp32command(cmd_str)

        # check page command
        self.assertIn("pageCommand", cmd)

        cmd = cmd["pageCommand"]

        # check individual values
        self.assertEqual(cmd["fileRequest"], "OPEN")
        self.assertEqual(cmd["fileDescriptor"], fd)
        self.assertEqual(cmd["blockSize"], bs)
        self.assertEqual(cmd["numBytes"], n)

    def test_page_req_not_implemented(self):
        """Test encoding a page command with a not implemented request"""

        with self.assertRaises(NotImplementedError):
            encode_esp32command("page", req="agg", fd=123, bs=456, n=789)

    def test_test_encode(self):
        """Test encoding a test command"""

        state = "receive"
        num = 123

        cmd_str = encode_esp32command("test", state=state, data=num)

        cmd = Esp32Command()
        cmd.ParseFromString(cmd_str)

        # check the command type
        cmd_type = cmd.WhichOneof("command")
        self.assertEqual(cmd_type, "test_command")

        # check individual values
        self.assertEqual(cmd.test_command.state, TestCommand.ChangeState.RECEIVE)
        self.assertEqual(cmd.test_command.data, num)

    def test_test_decode(self):
        """Test decoding a test command"""

        state = "receive"
        num = 123

        cmd_str = encode_esp32command("test", state=state, data=num)

        cmd = decode_esp32command(cmd_str)

        # check test command
        self.assertIn("testCommand", cmd)

        cmd = cmd["testCommand"]

        # check individual values
        self.assertEqual(cmd["state"], "RECEIVE")
        self.assertEqual(cmd["data"], num)

    def test_test_state_not_implemented(self):
        """Test encoding a page command with a not implemented state"""

        with self.assertRaises(NotImplementedError):
            encode_esp32command("test", state="agg", data=123)

    def test_wifi(self):
        """Test encoding/decoding a WiFi command

        All parameters are tested at once. This is not the intended
        implementation.
        """

        _type = "POST"
        ssid = "HelloWorld"
        passwd = "password"
        url = "https://test.com"
        port = 6969
        rc = 200
        ts = 1652346246
        resp = b"agga"

        # encode
        cmd_str = encode_esp32command(
            "wifi",
            _type=_type,
            ssid=ssid,
            passwd=passwd,
            url=url,
            port=port,
            rc=rc,
            ts=ts,
            resp=resp,
        )

        # decode
        cmd = decode_esp32command(cmd_str)

        # check the command key exists
        self.assertIn("wifiCommand", cmd)

        cmd = cmd["wifiCommand"]

        # check individual values
        self.assertEqual(cmd["type"], _type)
        self.assertEqual(cmd["ssid"], ssid)
        self.assertEqual(cmd["passwd"], passwd)
        self.assertEqual(cmd["url"], url)
        self.assertEqual(cmd["port"], port)
        self.assertEqual(cmd["rc"], rc)
        self.assertEqual(cmd["ts"], ts)
        self.assertEqual(base64.b64decode(cmd["resp"]), resp)

    def test_wifi_type_not_implemented(self):
        """Encode a WiFiCommand with a improper type to check for
        NotImplementedError
        """

        with self.assertRaises(NotImplementedError):
            encode_esp32command("wifi", _type="bla")


if __name__ == "__main__":
    unittest.main()
