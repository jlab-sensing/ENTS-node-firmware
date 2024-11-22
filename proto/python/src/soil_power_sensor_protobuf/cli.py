import argparse
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

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

    image_path = "../images/board_connection_calibration.png"
    img = mpimg.imread(image_path)
    plt.imshow(img)
    plt.axis('off')
    plt.title("Connect the Source Measure Unit to your ENTS board as illustrate below. \n\nCLOSE this window to START the calibration process.")
    plt.show()

    if run_v:
        record_calibrate(V_START, V_STOP, V_STEP, "voltage")

    if run_i:
        record_calibrate(I_START, I_STOP, I_STEP, "current")

    if args.plot:
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


def entry():
    """Entrypoint for command line interface"""

    parser = argparse.ArgumentParser(
        prog="Environmental NeTworked Sensor (ents) Utility "
    )
    sub_p = parser.add_subparsers(help="Ents Utilities")

    # calibration parser
    calib_p = sub_p.add_parser("calib", help="Calibrate power measurements")
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

    args = parser.parse_args()
    args.func(args)
