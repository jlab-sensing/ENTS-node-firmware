import argparse

import os
import pandas as pd
import numpy as np
from datetime import datetime, timedelta
import time

from .calibrate.recorder import Recorder
from .calibrate.linear_regression import (
    linear_regression,
    print_eval,
    print_coef,
    print_norm,
)
from .calibrate.plots import (
    plot_measurements,
    plot_calib,
    plot_residuals,
    plot_residuals_hist,
)

from .proto.encode import (
    encode_power_measurement,
    encode_phytos31_measurement,
    encode_teros12_measurement,
    encode_response,
)
from .proto.decode import decode_measurement, decode_response
from .proto.esp32 import encode_esp32command, decode_esp32command

from .proto.sensor import (
    encode_repeated_sensor_measurements,
    decode_repeated_sensor_measurements,
    encode_sensor_response,
    decode_sensor_response,
)

from .simulator.node import NodeSimulator


def entry():
    """Command line interface entry point"""

    parser = argparse.ArgumentParser()

    subparsers = parser.add_subparsers(help="Ents Utilities", required=True)

    create_encode_parser(subparsers)
    create_encode_generic_parser(subparsers)
    create_decode_parser(subparsers)
    create_decode_generic_parser(subparsers)
    create_calib_parser(subparsers)
    create_sim_parser(subparsers)
    create_sim_generic_parser(subparsers)

    args = parser.parse_args()
    args.func(args)


def create_sim_generic_parser(subparsers):
    """Creates the generic simulation subparser

    Args:
        subparsers: Reference to subparser group
    Returns:
        Reference to new subparser
    """

    sim_p = subparsers.add_parser("sim_generic", help="Simluate generic sensor uploads")
    sim_p.add_argument(
        "--url",
        required=True,
        type=str,
        help="URL of the dirtviz instance (default: http://localhost:8000)",
    )
    sim_p.add_argument(
        "--mode",
        required=True,
        choices=["batch", "stream"],
        type=str,
        help="Upload mode",
    )
    sim_p.add_argument(
        "--sensor",
        required=True,
        type=str,
        nargs="+",
        help="Type of sensor to simulate",
    )
    sim_p.add_argument("--min", type=float, default=-1., help="Minimum sensor value (default: -1.0)")
    sim_p.add_argument("--max", type=float, default=1., help="Maximum sensor value (default: 1.0)")
    sim_p.add_argument("--cell", required=True, type=int, help="Cell Id")
    sim_p.add_argument("--logger", required=True, type=int, help="Logger Id")
    sim_p.add_argument("--start", type=str, help="Start date")
    sim_p.add_argument("--end", type=str, help="End date")
    sim_p.add_argument(
        "--freq", default=10.0, type=float, help="Frequency of uploads (default: 10s)"
    )
    sim_p.set_defaults(func=simulate_generic)

    return sim_p


def create_sim_parser(subparsers):
    """Creates the simulation subparser

    Args:
        subparsers: Reference to subparser group
    Returns:
        Reference to new subparser
    """

    sim_p = subparsers.add_parser("sim", help="Simluate sensor uploads")
    sim_p.add_argument(
        "--url",
        required=True,
        type=str,
        help="URL of the dirtviz instance (default: http://localhost:8000)",
    )
    sim_p.add_argument(
        "--mode",
        required=True,
        choices=["batch", "stream"],
        type=str,
        help="Upload mode",
    )
    sim_p.add_argument(
        "--sensor",
        required=True,
        choices=["power", "teros12", "teros21", "bme280"],
        type=str,
        nargs="+",
    )
    sim_p.add_argument("--cell", required=True, type=int, help="Cell Id")
    sim_p.add_argument("--logger", required=True, type=int, help="Logger Id")
    sim_p.add_argument("--start", type=str, help="Start date")
    sim_p.add_argument("--end", type=str, help="End date")
    sim_p.add_argument(
        "--freq", default=10.0, type=float, help="Frequency of uploads (default: 10s)"
    )
    sim_p.set_defaults(func=simulate)

    return sim_p


def simulate_generic(args):
    simulation = NodeSimulator(
        cell=args.cell,
        logger=args.logger,
        sensors=args.sensor,
        _min=args.min,
        _max=args.max,
    )

    if args.mode == "batch":
        if (args.start is None) or (args.end is None):
            raise ValueError("Start and end date must be provided for batch mode.")

        # format dates
        curr_dt = datetime.fromisoformat(args.start)
        end_dt = datetime.fromisoformat(args.end)

        # create list of measurements
        while curr_dt <= end_dt:
            ts = int(curr_dt.timestamp())
            simulation.measure(ts)
            curr_dt += timedelta(seconds=args.freq)

        # send measurements
        while simulation.send_next(args.url):
            print(simulation)

        print("Done!")

    elif args.mode == "stream":
        print("Use CTRL+C to stop the simulation")
        try:
            while True:
                dt = datetime.now()
                ts = int(dt.timestamp())
                simulation.measure(ts)
                while simulation.send_next(args.url):
                    print(simulation)
                time.sleep(args.freq)
        except KeyboardInterrupt as _:
            print("Stopping simulation")


def simulate(args):
    simulation = NodeSimulator(
        cell=args.cell,
        logger=args.logger,
        sensors=args.sensor,
    )

    if args.mode == "batch":
        if (args.start is None) or (args.end is None):
            raise ValueError("Start and end date must be provided for batch mode.")

        # format dates
        curr_dt = datetime.fromisoformat(args.start)
        end_dt = datetime.fromisoformat(args.end)

        # create list of measurements
        while curr_dt <= end_dt:
            ts = int(curr_dt.timestamp())
            simulation.measure(ts)
            curr_dt += timedelta(seconds=args.freq)

        # send measurements
        while simulation.send_next(args.url):
            print(simulation)

        print("Done!")

    elif args.mode == "stream":
        print("Use CTRL+C to stop the simulation")
        try:
            while True:
                dt = datetime.now()
                ts = int(dt.timestamp())
                simulation.measure(ts)
                while simulation.send_next(args.url):
                    print(simulation)
                time.sleep(args.freq)
        except KeyboardInterrupt as _:
            print("Stopping simulation")




def create_calib_parser(subparsers):
    """Creates the calibration subparser

    Args:
        subparsers: Reference to subparser group
    Returns:
        Reference to new subparser
    """

    # calibration parser
    calib_p = subparsers.add_parser("calib", help="Calibrate power measurements")
    calib_p.add_argument(
        "--samples",
        type=int,
        default=10,
        required=False,
        help="Samples taken at each step (default: 10)",
    )
    calib_p.add_argument(
        "--plot", action="store_true", help="Show calibration parameter plots"
    )
    calib_p.add_argument(
        "--mode",
        type=str,
        default="both",
        required=False,
        help="Either both, voltage, or current (default: both)",
    )
    calib_p.add_argument(
        "--output", type=str, required=False, help="Output directory for measurements"
    )
    calib_p.add_argument("port", type=str, help="Board serial port")
    calib_p.add_argument("host", type=str, help="Address and port of smu (ip:port)")
    calib_p.set_defaults(func=calibrate)

    return calib_p


def create_encode_generic_parser(subparsers):
    """Create generic encode command subparser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """

    encode_parser = subparsers.add_parser(
        "encode_generic", help="Encode generic data"
    )

    print_format = encode_parser.add_mutually_exclusive_group()
    print_format.add_argument(
        "--hex", action="store_true", help="Print as hex values (default)"
    )
    print_format.add_argument(
        "--raw", action="store_true", help="Print raw bytes object"
    )
    print_format.add_argument(
        "--c", action="store_true", help="Print bytes for copying to c"
    )

    encode_subparsers = encode_parser.add_subparsers(title="Message type",
                                                     dest="type",
                                                     required=True)

    # sensor measurement
    measurement_parser = encode_subparsers.add_parser(
        "meas", help='Proto "Measurement" message'
    )
    measurement_parser.add_argument("--ts", type=int, help="Unix epoch timestamp")
    measurement_parser.add_argument("--cell", type=int, help="Cell Id")
    measurement_parser.add_argument("--logger", type=int, help="Logger Id")
    measurement_parser.add_argument("--idx", type=int, default=1, help="Upload index")
    measurement_parser.add_argument(
        "--sensor",
        nargs=2,
        metavar=("type", "value"),
        action="append",
        required=True,
        help="Specify as: --sensor <type> <value>",
    )
    measurement_parser.set_defaults(func=handle_encode_generic_measurement)

    # response
    response_parser = encode_subparsers.add_parser(
        "resp", help='Proto "Response" message'
    )
    response_parser.add_argument(
        "--resp",
        nargs=2,
        metavar=("idx", "status"),
        action="append",
        required=True,
        help="Specify as: --resp <idx> <status>",
    )
    response_parser.set_defaults(func=handle_encode_generic_response)

    return encode_parser


def parse_number(s: str) -> tuple:
    try:
        i = int(s)
        if i >= 0:
            return i, 'unsignedInt'
        return i, 'signedInt'
    except ValueError:
        pass

    try:
        return float(s), 'decimal'
    except ValueError:
        pass

    raise ValueError(f"Invalid numeric value: {s}")


def handle_encode_generic_measurement(args):
    """Take arguments and encode a repeated senosr measurement message"""

    meas = {
        "meta": {
            "ts": args.ts,
            "cellId": args.cell,
            "loggerId": args.logger,
        },

        "measurements": [],
    }

    for s in args.sensor:
        val, val_type = parse_number(s[1])
        meas["measurements"].append(
            {
                "type": s[0],
                val_type: val,
                "idx": args.idx,
            }
        )
        args.idx += 1

    data = encode_repeated_sensor_measurements(meas)
    print_data(args, data)

def handle_encode_generic_response(args):
    """Takes arguments and encode a sensor response message"""

    resp = {
        "responses": [],
    }

    for r in args.resp:
        idx = int(r[0])
        status = r[1]
        resp["responses"].append(
            {
                "uploadIndex": idx,
                "status": status,
            }
        )

    data = encode_sensor_response(resp)
    print_data(args, data)

def create_encode_parser(subparsers):
    """Create encode command subparser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """

    encode_parser = subparsers.add_parser("encode", help="Encode data")

    print_format = encode_parser.add_mutually_exclusive_group()
    print_format.add_argument(
        "--hex", action="store_true", help="Print as hex values (default)"
    )
    print_format.add_argument(
        "--raw", action="store_true", help="Print raw bytes object"
    )
    print_format.add_argument(
        "--c", action="store_true", help="Print bytes for copying to c"
    )

    encode_subparsers = encode_parser.add_subparsers(
        title="Message type",
        dest="type",
        required=True,
    )

    def create_measurement_parser(encode_subparsers):
        measurement_parser = encode_subparsers.add_parser(
            "measurement", help='Proto "Measurement" message'
        )
        measurement_subparser = measurement_parser.add_subparsers(
            title="Measurement type",
            required=True,
        )

        # metadata
        measurement_parser.add_argument("ts", type=int, help="Unix epoch timestamp")
        measurement_parser.add_argument("cell", type=int, help="Cell Id")
        measurement_parser.add_argument("logger", type=int, help="Logger Id")

        power_parser = measurement_subparser.add_parser(
            "power", help="PowerMeasurement"
        )
        power_parser.add_argument("voltage", type=float, help="Voltage in (V)")
        power_parser.add_argument("current", type=float, help="Current in (A)")
        power_parser.set_defaults(func=handle_encode_measurement_power)

        teros12_parser = measurement_subparser.add_parser(
            "teros12", help="Teros12Measurement"
        )
        teros12_parser.add_argument("vwc_raw", type=float, help="Raw vwc")
        teros12_parser.add_argument("vwc_adj", type=float, help="Calibrated vwc")
        teros12_parser.add_argument("temp", type=float, help="Temperature in C")
        teros12_parser.add_argument("ec", type=int, help="Electrical conductivity")
        teros12_parser.set_defaults(func=handle_encode_measurement_teros12)

        phytos31_parser = measurement_subparser.add_parser(
            "phytos31", help="Phytos31Measurement"
        )
        phytos31_parser.add_argument("voltage", type=float, help="Raw voltage (V)")
        phytos31_parser.add_argument("leaf_wetness", type=float, help="Leaf wetness")
        phytos31_parser.set_defaults(func=handle_encode_measurement_phytos31)

        return measurement_parser

    def create_response_parser(encode_subparsers):
        response_parser = encode_subparsers.add_parser(
            "response", help='Proto "Response" message'
        )
        response_parser.add_argument("status", type=str, help="Status")
        response_parser.set_defaults(func=handle_encode_response)

        return response_parser

    def create_esp32command_parser(encode_subparsers):
        esp32command_parser = encode_subparsers.add_parser(
            "esp32command", help='Proto "Esp32Command" message'
        )
        esp32command_subparser = esp32command_parser.add_subparsers(
            title="type",
            help="PageCommand",
            required=True,
        )

        test_parser = esp32command_subparser.add_parser("test", help="TestCommand")
        test_parser.add_argument("state", type=str, help="State to put module into")
        test_parser.add_argument("data", type=int, help="Data associated with command")
        test_parser.set_defaults(func=handle_encode_esp32command_test)

        esp32_parser = esp32command_subparser.add_parser("page", help="PageCommand")
        esp32_parser.add_argument("type", type=str, help="Request type")
        esp32_parser.add_argument("fd", type=int, help="File descriptor")
        esp32_parser.add_argument("bs", type=int, help="Block size")
        esp32_parser.add_argument("num", type=int, help="Number of bytes")
        esp32_parser.set_defaults(func=handle_encode_esp32command_page)

        wifi_parser = esp32command_subparser.add_parser("wifi", help="WiFiCommand")
        wifi_parser.add_argument("type", type=str, help="WiFi command type")
        wifi_parser.add_argument("--ssid", type=str, default="", help="WiFi SSID")
        wifi_parser.add_argument("--passwd", type=str, default="", help="WiFi password")
        wifi_parser.add_argument("--url", type=str, default="", help="Endpoint url")
        wifi_parser.add_argument("--port", type=int, default=0, help="Endpoint port")
        wifi_parser.add_argument("--rc", type=int, default=0, help="Return code")
        wifi_parser.add_argument("--ts", type=int, help="Timestamp in unix epochs")
        wifi_parser.add_argument(
            "--resp", type=str, default=b"", help="Serialized response message"
        )
        wifi_parser.set_defaults(func=handle_encode_esp32command_wifi)

        return esp32command_parser

    # create subparsers
    create_measurement_parser(encode_subparsers)
    create_response_parser(encode_subparsers)
    create_esp32command_parser(encode_subparsers)

    return encode_parser


def handle_encode_measurement_power(args):
    data = encode_power_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        voltage=args.voltage,
        current=args.current,
    )

    print_data(args, data)


def handle_encode_measurement_teros12(args):
    data = encode_teros12_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        vwc_raw=args.vwc_raw,
        vwc_adj=args.vwc_adj,
        temp=args.temp,
        ec=args.ec,
    )

    print_data(args, data)


def handle_encode_measurement_phytos31(args):
    data = encode_phytos31_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        voltage=args.voltage,
        leaf_wetness=args.leaf_wetness,
    )

    print_data(args, data)


def handle_encode_response(args):
    valid_status = ["SUCCESS", "ERROR"]
    if args.status not in valid_status:
        raise NotImplementedError(f'Response status "{args.status}" not implemented')

    data = encode_response(args.status)
    print_data(args, data)


def handle_encode_esp32command_test(args):
    data = encode_esp32command("test", state=args.state, data=args.data)
    print_data(args, data)


def handle_encode_esp32command_page(args):
    data = encode_esp32command(
        "page", req=args.type.lower(), fd=args.fd, bs=args.bs, n=args.num
    )
    print_data(args, data)


def handle_encode_esp32command_wifi(args):
    data = encode_esp32command(
        "wifi",
        _type=args.type.lower(),
        ssid=args.ssid,
        passwd=args.passwd,
        url=args.url,
        port=args.port,
        rc=args.rc,
        ts=args.ts,
        resp=args.resp,
    )
    print_data(args, data)


def print_data(args, data: bytes) -> str:
    if args.c:
        print_bytes_c(data)
    elif args.raw:
        print(data)
    else:
        print(data.hex())


def print_bytes_c(data: bytes) -> str:
    """Formats serialized data into c bytes array"""

    # format data string
    data_str = "uint8_t data[] = {"
    hex_str = [hex(d) for d in data]
    data_str += ", ".join(hex_str)
    data_str += "};"

    # print data string
    print(data_str)

    # print length of data string
    print(f"size_t data_len = {len(hex_str)};")


def create_decode_generic_parser(subparsers):
    """Create generic decode command parser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """

    decode_parser = subparsers.add_parser("decode_generic", help="Decode generic data")

    decode_subparsers = decode_parser.add_subparsers(
        title="Message type",
        dest="type",
        required=True,
    )

    decode_parser.add_argument(
        "data", type=str, help="Protobuf serialized data in hex format"
    )

    # sensor measurement
    measurement_parser = decode_subparsers.add_parser(
        "meas", help='Proto "Measurement" message'
    )
    measurement_parser.set_defaults(func=handle_decode_generic_measurement)

    # response
    response_parser = decode_subparsers.add_parser(
        "resp", help='Proto "Response" message'
    )
    response_parser.set_defaults(func=handle_decode_generic_response)

    return decode_parser


def handle_decode_generic_measurement(args):
    data = bytes.fromhex(args.data)
    vals = decode_repeated_sensor_measurements(data)
    print(vals)


def handle_decode_generic_response(args):
    data = bytes.fromhex(args.data)
    vals = decode_sensor_response(data)
    print(vals)


def create_decode_parser(subparsers):
    """Create decode command parser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """

    decode_parser = subparsers.add_parser("decode", help="Decode data")

    decode_subparsers = decode_parser.add_subparsers(
        title="Message type",
        dest="type",
        required=True,
    )

    # measurement
    measurement_parser = decode_subparsers.add_parser(
        "measurement", help='Proto "Measurement" message'
    )
    measurement_parser.set_defaults(func=handle_decode_measurement)

    # response
    response_parser = decode_subparsers.add_parser(
        "response", help='Proto "Response" message'
    )
    response_parser.set_defaults(func=handle_decode_response)

    # esp32command
    esp32command_parser = decode_subparsers.add_parser(
        "esp32command", help='Proto "Esp32Command" message'
    )
    esp32command_parser.set_defaults(func=handle_decode_esp32command)

    decode_parser.add_argument(
        "data", type=str, help="Protobuf serialized data in hex format"
    )

    return decode_parser


def handle_decode_measurement(args):
    data = bytes.fromhex(args.data)
    vals = decode_measurement(data)
    print(vals)


def handle_decode_response(args):
    data = bytes.fromhex(args.data)
    vals = decode_response(data)
    print(vals)


def handle_decode_esp32command(args):
    data = bytes.fromhex(args.data)
    vals = decode_esp32command(data)
    print(vals)


def calibrate(args):
    print(
        "If you don't see any output for 5 seconds, restart the calibration after resetting the ents board"
    )

    host, port = args.host.split(":")
    rec = Recorder(args.port, host, int(port))

    if args.mode == "both":
        run_v = True
        run_i = True
    elif args.mode in ["v", "volts", "voltage"]:
        run_v = True
        run_i = False
    elif args.mode in ["i", "amps", "current"]:
        run_v = True
        run_i = True
    else:
        raise NotImplementedError(f"Calbration mode: {args.mode} not implemented")

    V_START = -2.0
    V_STOP = 2.0
    V_STEP = 0.5

    I_START = -0.0009
    I_STOP = 0.0009
    I_STEP = 0.00045

    def record_calibrate(start, stop, step, name: str):
        """Record and calibrate

        Args:
            start: Start value
            stop: Stop value (inclusive)
            step: Step between values
            name: Name of channel
        """

        # TODO Unjank reference to member variables by moving the selection to
        # the class.
        if name == "voltage":
            iterator = Recorder.record_voltage
        elif name == "current":
            iterator = Recorder.record_current

        # collect data
        print("Collecting calibration data")
        cal = iterator(rec, start, stop, step, args.samples)
        if args.output:
            save_csv(cal, args.output, f"{name}-cal.csv")

        print("Collecting evaluation data")
        _eval = iterator(rec, start, stop, step, args.samples)
        if args.output:
            save_csv(_eval, args.output, f"{name}-eval.csv")

        model = linear_regression(
            np.array(cal["meas"]).reshape(-1, 1), np.array(cal["actual"]).reshape(-1, 1)
        )
        pred = model.predict(np.array(_eval["meas"]).reshape(-1, 1))
        residuals = np.array(_eval["actual"]) - pred.flatten()

        print("")
        print("\r\rnCoefficients")
        print_coef(model)
        print("\r\nEvaluation")
        print_eval(pred, _eval["actual"])
        print("\r\nNormal fit")
        print_norm(residuals)
        print("")

        # plots
        if args.plot:
            plot_measurements(cal["actual"], cal["meas"], title=name)
            plot_calib(_eval["meas"], pred, title=name)
            plot_residuals(pred, residuals, title=name)
            plot_residuals_hist(residuals, title=name)

    if run_v:
        print("Connect smu to voltage inputs device and press ENTER")
        input()
        record_calibrate(V_START, V_STOP, V_STEP, "voltage")

    if run_i:
        print(
            "Connect smu to a resistor in series with the current channels and press ENTER"
        )
        input()
        record_calibrate(I_START, I_STOP, I_STEP, "current")

    print("Press enter to close plots")
    input()


def save_csv(data: dict[str, list], path: str, name: str):
    """Save measurement dictionary to csv

    Args:
        data: Measurement data
        path: Folder path
        name: Name of csv file
    """
    path = os.path.join(path, name)
    pd.DataFrame(data).to_csv(path, index=False)
