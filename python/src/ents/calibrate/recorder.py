#!/usr/bin/env python

"""Data recorder for Soil Power Sensor

The recorder controls the SPS firmware and a Keithley 2400 Source
Measurement Unit (SMU). Ideally the script should work with any microcontrollre
flashed with the firmware and any source measurement unit that supports
Standard Commands for Programable Instruments (SCPI). The units listed are the
ones that the script was developed and tested on. It allows to step through
a range of output voltages on the Keithley and measure the voltage and current
from both the SMU and the Soil Power Sensor (SPS).
"""

import time
import socket
import serial
from typing import Tuple
from tqdm import tqdm
from ..proto.sensor import decode_repeated_sensor_measurements


class SerialController:
    """Generic serial controller that will open and close serial connections"""

    # Serial port
    ser = None

    def __init__(self, port, baudrate=115200, xonxoff=False):
        """Constructor

        Initialises connection to serial port.

        Parameters
        ----------
        port : str
            Serial port of device
        baudrate : int, optional
            Baud rate for serial communication (default is 115200, STM32 functions at 115200)
        xonxoff  : bool, optional
            Flow control (default is on)
        """

        self.ser = serial.Serial(port, baudrate=baudrate, xonxoff=xonxoff)

    def __del__(self):
        """Destructor

        Closes connection to serial port.
        """

        self.ser.close()


class LANController:
    """Generic LAN controller that will open and close LAN connections"""

    # Socket
    sock = None

    def __init__(self, host, port):
        """Constructor

        Initialises connection to LAN device.

        Parameters
        ----------
        host : str
            IP address or hostname of the device
        port : int
            Port number of the device
        """

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

    def __del__(self):
        """Destructor

        Closes connection to LAN device.
        """

        self.sock.close()


class SoilPowerSensorController(SerialController):
    """Controller used to read values from the SPS"""

    def __init__(self, port):
        """Constructor

        Opens serial connection and checks functionality

        Parameters
        ----------
        port : str
            Serial port of device
        """
        super().__init__(port)
        self.check()

    def get_power(self) -> Tuple[float, float]:
        """Measure voltage from SPS

        Returns
        -------
        tuple[float, float
            voltage, current
        """

        self.ser.write(b"0")  # send a command to the SPS to send a power measurment

        # read a single byte for the length
        resp_len_bytes = self.ser.read()

        resp_len = int.from_bytes(resp_len_bytes)

        reply = self.ser.read(resp_len)  # read said measurment

        meas_dict = decode_repeated_sensor_measurements(reply)  # decode using protobuf

        voltage_value = meas_dict["measurements"][0]["decimal"]
        current_value = meas_dict["measurements"][1]["decimal"]

        return float(voltage_value), float(current_value)

    def check(self):
        """Performs a check of the connection to the board

        Raises
        ------
        RuntimeError
            Checks that SPS replies "ok" when sent "check"
        """

        # needed sleep to get write to work
        # possibly due to linux usb stack initialized or mcu waiting to startup
        time.sleep(1)
        self.ser.write(b"check\n")

        reply = self.ser.readline()
        # reply = reply.decode()
        # reply = reply.strip("\r\n")

        if reply != b"ok\n":
            raise RuntimeError(f"SPS check failed. Reply received: {reply}")


class SMUSerialController(SerialController):
    """Controller for the Keithley 2400 SMU used to supply known voltage to the
    SPS

    Uses SCPI (Standard Control of Programmable Instruments) to control the SMU
    through its RS232 port. Written for the Keithley 2400 SMU, but should work
    for any other SMU that uses SCPI.
    """

    class VoltageIterator:
        """VoltageIterator Class

        Implements a iterator for looping through voltage output values
        """

        def __init__(self, ser, start, stop, step):
            """Constructor

            Parameters
            ----------
            ser : serial.Serial
                Initialised serial connection
            start : float
                Starting voltage
            stop : float
                End voltage
            step : float
                Voltage step
            """

            self.ser = ser
            self.start = start
            self.stop = stop
            self.step = step

        def __iter__(self):
            """Iterator

            Sets current value to start
            """

            self.v = None
            self.ser.write(b":OUTP ON\n")
            return self

        def __next__(self):
            """Next

            Steps to next voltage level, stopping once stop is reached

            Raises
            ------
            StopIteration
                When the next step exceeds the stop level
            """

            if self.v is None:
                return self.set_voltage(self.start)

            v_next = self.v + self.step

            if v_next <= self.stop:
                return self.set_voltage(v_next)
            else:
                raise StopIteration

        def set_voltage(self, v):
            """Sets the voltage output"""

            self.v = v
            cmd = f":SOUR:VOLT:LEV {v}\n"
            self.ser.write(bytes(cmd, "ascii"))
            return self.v

    def __init__(self, port, source_mode):
        """Constructor

        Opens serial port, restore to known defaults

        Parameters
        ----------
        port : str
            Serial port of device
        """

        super().__init__(port)
        # Reset settings
        self.ser.write(b"*RST\n")
        # Voltage source
        self.ser.write(b":SOUR:FUNC VOLT\n")
        self.ser.write(b":SOUR:VOLT:MODE FIXED\n")
        # 1mA compliance
        self.ser.write(b":SENS:CURR:PROT 10e-3\n")
        # Sensing functions
        self.ser.write(b":SENS:CURR:RANGE:AUTO ON\n")
        self.ser.write(b":SENS:FUNC:OFF:ALL\n")
        self.ser.write(b':SENS:FUNC:ON "VOLT"\n')
        self.ser.write(b':SENS:FUNC:ON "CURR"\n')

    def __del__(self):
        """Destructor

        Turns off output
        """

        self.ser.write(b":OUTP OFF\n")

    def vrange(self, start, stop, step) -> VoltageIterator:
        """Gets iterator to range of voltages

        Parameters
        ----------
        start : float
            Starting voltage
        stop : float
            End voltage
        step : float
            Voltage step
        """

        return self.VoltageIterator(self.ser, start, stop, step)

    def get_voltage(self) -> float:
        """Measure voltage supplied to the SPS from SMU

        Returns
        -------
        float
            Measured voltage
        """

        self.ser.write(b":FORM:ELEM VOLT\n")
        self.ser.write(b":READ?\n")
        reply = self.ser.readline().decode()
        reply = reply.strip("\r")
        return float(reply)

    def get_current(self) -> float:
        """Measure current supplied to the SPS from SMU

        Returns
        -------
        float
            Measured current
        """

        self.ser.write(
            b":FORM:ELEM CURR\n"
        )  # replace with serial.write with socket.write commands, std library aviable, lots of example code
        self.ser.write(b":READ?\n")
        reply = self.ser.readline().decode()
        reply = reply.strip("\r")
        return float(reply)


class SMULANController(LANController):
    """Controller for the Keithley 2400 SMU used to supply known voltage to the
    SPS

    Uses SCPI (Standard Control of Programmable Instruments) to control the SMU
    through its RS232 port. Written for the Keithley 2400 SMU, but should work
    for any other SMU that uses SCPI.
    """

    class VoltageIterator:
        """VoltageIterator Class

        Implements a iterator for looping through voltage output values
        """

        def __init__(self, sock, start, stop, step):
            """Constructor

            Parameters
            ----------
            sock : serial.Socket
                Initialised socket connection
            start : float
                Starting voltage
            stop : float
                End voltage
            step : float
                Voltage step
            """

            self.sock = sock
            self.start = start
            self.stop = stop
            self.step = step

        def __iter__(self):
            """Iterator

            Sets current value to start
            """

            self.v = None
            self.sock.sendall(b":OUTP ON\n")
            return self

        def __next__(self):
            """Next

            Steps to next voltage level, stopping once stop is reached

            Raises
            ------
            StopIteration
                When the next step exceeds the stop level
            """

            if self.v is None:
                return self.set_voltage(self.start)

            v_next = self.v + self.step

            if v_next <= self.stop:
                return self.set_voltage(v_next)
            else:
                raise StopIteration

        def __len__(self):
            """Len

            The number of measurements points
            """
            return int((self.stop - self.start) / self.step) + 1

        def set_voltage(self, v):
            """Sets the voltage output"""

            self.v = v
            cmd = f":SOUR:VOLT:LEV {v}\n"
            self.sock.sendall(bytes(cmd, "ascii"))
            return self.v

    class CurrentIterator:
        """CurrentIterator Class

        Implements a iterator for looping through current output values
        """

        def __init__(self, sock, start, stop, step):
            """Constructor

            Parameters
            ----------
            sock : serial.Socket
                Initialised socket connection
            start : float
                Starting current
            stop : float
                End current
            step : float
                Current step
            """
            self.sock = sock
            self.start = start
            self.stop = stop
            self.step = step

        def __iter__(self):
            """Iterator

            Sets current value to start
            """

            self.i = None
            self.sock.sendall(b":OUTP ON\n")
            return self

        def __next__(self):
            """Next

            Steps to next voltage level, stopping once stop is reached

            Raises
            ------
            StopIteration
                When the next step exceeds the stop level
            """

            if self.i is None:
                return self.set_current(self.start)
            i_next = self.i + self.step
            if i_next <= self.stop:
                return self.set_current(i_next)
            else:
                raise StopIteration

        def __len__(self):
            """Len

            The number of measurements points
            """
            return int((self.stop - self.start) / self.step) + 1

        def set_current(self, i):
            """Sets the current output"""

            self.i = i
            cmd = f":SOUR:CURR:LEV {i}\n"
            self.sock.sendall(bytes(cmd, "ascii"))
            return self.i

    def __init__(self, host, port):
        """Constructor

        Opens LAN connection and sets initial SMU configurations.

        Parameters
        ----------
        host : str
            IP address or hostname of the SMU
        port : int
            Port number used for the LAN connection
        """

        super().__init__(host, port)

    def setup_voltage(self):
        """Configures smu for sourcing voltage"""

        self.sock.sendall(b"*RST\n")
        # Voltage source
        self.sock.sendall(b":SOUR:FUNC VOLT\n")
        self.sock.sendall(b":SOUR:VOLT:MODE FIXED\n")
        # 1mA compliance
        self.sock.sendall(b":SENS:CURR:PROT 10e-3\n")
        # Sensing functions
        self.sock.sendall(b":SENS:CURR:RANGE:AUTO ON\n")
        self.sock.sendall(b":SENS:FUNC:OFF:ALL\n")
        self.sock.sendall(b':SENS:FUNC:ON "VOLT"\n')
        self.sock.sendall(b':SENS:FUNC:ON "CURR"\n')

    def setup_current(self):
        """Configured smu for sourcing current"""

        self.sock.sendall(b"*RST\n")
        # Current source
        self.sock.sendall(b":SOUR:FUNC CURR\n")
        self.sock.sendall(b":SOUR:CURR:MODE FIXED\n")
        # 1V compliance
        self.sock.sendall(b":SENSE:CURR:PROT 1\n")
        # Sensing functions
        self.sock.sendall(b":SENS:VOLT:RANGE:AUTO ON\n")
        self.sock.sendall(b":SENS:FUNC:OFF:ALL\n")
        self.sock.sendall(b':SENS:FUNC:ON "CURR"\n')
        self.sock.sendall(b':SENS:FUNC:ON "VOLT"\n')

    def __del__(self):
        """Destructor

        Turns off output
        """

        self.sock.sendall(b":OUTP OFF\n")

    def vrange(self, start, stop, step) -> VoltageIterator:
        """Gets iterator to range of voltages

        Parameters
        ----------
        start : float
            Starting voltage
        stop : float
            End voltage
        step : float
            Voltage step
        """

        self.setup_voltage()
        return self.VoltageIterator(self.sock, start, stop, step)

    def irange(self, start, stop, step) -> CurrentIterator:
        """Gets iterator to range of currents

        Parameters
        ----------
        start : float
            Starting current
        stop : float
            End current
        step : float
            Current step
        """

        self.setup_current()
        return self.CurrentIterator(self.sock, start, stop, step)

    def get_voltage(self) -> float:
        """Measure voltage supplied to the SPS from SMU

        Returns
        -------
        float
            Measured voltage
        """

        self.sock.sendall(b":FORM:ELEM VOLT\n")
        self.sock.sendall(b":READ?\n")
        # receive response
        reply = self.sock.recv(256)
        # strip trailing \r\n characters
        reply = reply.strip()
        # convert to float and return
        return float(reply)

    def get_current(self) -> float:
        """Measure current supplied to the SPS from SMU

        Returns
        -------
        float
            Measured current
        """

        self.sock.sendall(
            b":FORM:ELEM CURR\n"
        )  # replace with serial.write with socket.write commands, std library aviable, lots of example code
        self.sock.sendall(b":READ?\n")
        # receive response
        reply = self.sock.recv(256)
        # strip trailing \r\n characters
        reply = reply.strip()
        # convert to float and return
        return float(reply)


class Recorder:
    def __init__(self, serialport: str, host: str, port: int, delay: int = 1):
        """Recorder constructor

        Initializes the connection to smu and sps.

        Args:
            serialport: Serial port to sps
            host: Hostname/ip of smu
            port: TCP port to smu
            delay: Delay between smu step and power measurement in seconds
        """

        self.sps = SoilPowerSensorController(serialport)
        self.smu = SMULANController(host, port)
        self.delay = delay

    def record_voltage(
        self, start: float, stop: float, step: float, samples: int
    ) -> dict[str, list]:
        """Records voltage measurements from the smu and sps

        Input arguments given in volts units. Returned dictionary has keys for the
        expected voltage (number from python), actual voltage (value measured by
        the smu), and meas voltage (value measured by the sps).

        Args:
            start: Start voltage
            stop: Stop voltage (inclusive)
            step: Step between voltage
            samples: Number of samples taken at each voltage

        Returns:
            Dictionary in the following pandas compatable format:
            {
                "expected": [],
                "actual": [],
                "meas": [],
            }
        """

        data = {
            "expected": [],
            "actual": [],
            "meas": [],
        }

        for value in tqdm(self.smu.vrange(start, stop, step)):
            time.sleep(self.delay)
            for _ in range(samples):
                # expected
                data["expected"].append(value)
                # smu
                data["actual"].append(self.smu.get_voltage())
                # sps
                sps_v, _ = self.sps.get_power()
                data["meas"].append(sps_v)

        return data

    def record_current(
        self, start: float, stop: float, step: float, samples: int
    ) -> dict[str, list]:
        """Records current measurements from the smu and sps

        Input arguments given in amps units. Returned dictionary has keys for the
        expected current (number from python), actual current (value measured by
        the smu), and meas current (value measured by the sps).

        Args:
            start: Start current
            stop: Stop current (inclusive)
            step: Step between current
            samples: Number of samples taken at each voltage

        Returns:
            Dictionary in the following pandas compatable format:
            {
                "expected": [],
                "actual": [],
                "meas": [],
            }
        """

        data = {
            "expected": [],
            "actual": [],
            "meas": [],
        }

        for value in tqdm(self.smu.irange(start, stop, step)):
            time.sleep(self.delay)
            for _ in range(samples):
                # expected
                data["expected"].append(value)
                # smu
                data["actual"].append(self.smu.get_current())
                # sps
                _, sps_i = self.sps.get_power()
                data["meas"].append(sps_i)

        return data
