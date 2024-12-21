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

import tkinter as tk
from tkinter import Label, Entry, Button
from PIL import Image, ImageTk

def start_calibration():
    board_id = board_id_entry.get()  # Get the user input from the entry field
    print(f"Board ID: {board_id}")  # Print or use the Board ID as needed
    root.destroy()  # Close the window after the user submits

# Create the main window
root = tk.Tk()
root.title("Board Connection Calibration")

# Load the image
image_path = "../images/board_connection_calibration.png"
img = Image.open(image_path)
img = img.resize((600, 400))  # Resize the image to fit the window
photo = ImageTk.PhotoImage(img)

# Create and place the image label
image_label = Label(root, image=photo)
image_label.pack(pady=10)

# Add instructions
instructions = Label(root, text="Connect the Source Measure Unit to your ENTS board as illustrated below.\n\nEnter the Board ID to start calibration.", font=("Arial", 12))
instructions.pack(pady=10)

# Add the entry field for Board ID
board_id_entry = Entry(root, font=("Arial", 14))
board_id_entry.pack(pady=5)

# Add a submit button
submit_button = Button(root, text="Start Calibration", command=start_calibration, font=("Arial", 12))
submit_button.pack(pady=10)

# Run the Tkinter event loop
root.mainloop()



def save_results_to_excel(results, output_file="results.xlsx"):
    """
    Save the results dictionary to an Excel file.

    Args:
        results (dict): The results dictionary from record_calibrate.
        output_file (str): Path to save the Excel file.
    """
    # Create a writer object
    with pd.ExcelWriter(output_file, engine="openpyxl") as writer:
        # Save calibration data
        cal_df = pd.DataFrame(results["calibration_data"])
        cal_df.to_excel(writer, sheet_name="Calibration Data", index=False)

        # Save evaluation data
        eval_df = pd.DataFrame(results["evaluation_data"])
        eval_df.to_excel(writer, sheet_name="Evaluation Data", index=False)

        # Save residuals
        residuals_df = pd.DataFrame({
            "Actual": results["evaluation"]["actual"],
            "Predicted": results["evaluation"]["predicted"],
            "Residuals": results["residuals"]
        })
        residuals_df.to_excel(writer, sheet_name="Residuals", index=False)

        # Save model coefficients and intercept
        coef_df = pd.DataFrame({
            "Coefficient": results["model"].coef_.flatten(),
            "Intercept": [results["model"].intercept_]
        })
        coef_df.to_excel(writer, sheet_name="Model Coefficients", index=False)

    print(f"Results saved to {output_file}")

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

    V_START = 0.0
    V_STOP = 2.0
    V_STEP = 1.0

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

        results = {
            "calibration_data": cal,
            "evaluation_data": _eval,
            "model": model,  # Include the full model object
            "predictions": pred,  # Predicted values
            "residuals": residuals,  # Residuals (actual - predicted)
            "evaluation": {
                "actual": _eval["actual"],  # Actual evaluation data
                "predicted": pred.flatten(),  # Flattened predictions for compatibility
            },
        }

        # print("")
        # print("\r\rnCoefficients")
        # print_coef(model)
        # print("\r\nEvaluation")
        # print_eval(pred, _eval["actual"])
        # print("\r\nNormal fit")
        # print_norm(residuals)
        # print("")

        # plots
        if args.plot:
            plot_measurements(cal["actual"], cal["meas"], title=name)
            plot_calib(_eval["meas"], pred, title=name)
            plot_residuals(pred, residuals, title=name)
            plot_residuals_hist(residuals, title=name)

        return results

    # image_path = "../images/board_connection_calibration.png"
    # img = mpimg.imread(image_path)
    # plt.imshow(img)
    # plt.axis('off')
    # plt.title("Connect the Source Measure Unit to your ENTS board as illustrate below. \n\nCLOSE this window to START the calibration process.")
    # plt.show()

    if run_v:
        results_v = record_calibrate(V_START, V_STOP, V_STEP, "voltage")

        model = results_v['model']  # Assuming the model is returned in the results
        pred = results_v['predictions']
        actual = results_v["evaluation"]["actual"]
        residuals = results_v["residuals"]

        save_results_to_excel(results_v, output_file="voltage_calibration_results.xlsx")

        print("\nCoefficients")
        print_coef(model)
        print("\nEvaluation")
        print_eval(pred, actual)
        #print("\nResiduals")
        #print(residuals)

    # if run_i:
    #     record_calibrate(I_START, I_STOP, I_STEP, "current")

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
