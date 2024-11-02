#!/usr/bin/env python

"""Data recorder for Soil Power Sensor

The recorder controls the SPS firmware and a Keithley 2400 Source
Measurement Unit (SMU). Ideally the script should work with any microcontrollre
flashed with the firmware and any source measurement unit that supports
Standard Commands for Programable Instruments (SCPI). The units listed are the
ones that the script was developed and tested on. It allows to step through
a range of output voltages on the Keithley and measure the voltage and current
from both the SMU and the Soil Power Sensor (SPS).

Example
-------
The following examples shows how to collect data on voltage ranges 0-3.3V
increasing the voltage by 0.1V every step. The SPS is connected to
``/dev/ttyACM0`` and the SMu is connected to ``/dev/ttyUSB0``.::

    $ python recorder.py 0 3.3 0.1 /dev/ttyACM0 /dev/ttyACM0

To see a list of all CLI parameters:::

    $ python recorder.py -h
"""


import time
import argparse
import socket
import serial
import numpy as np
import pandas as pd
import pdb
from tqdm import tqdm
from typing import Tuple
from soil_power_sensor_protobuf import decode_measurement


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
        # Print serial port settings
        print("Serial Port Settings:")
        print("Port:", self.ser.port)
        print("Baudrate:", self.ser.baudrate)
        print("Byte size:", self.ser.bytesize)
        print("Parity:", self.ser.parity)
        print("Stop bits:", self.ser.stopbits)
        print("Timeout:", self.ser.timeout)
        print("Xon/Xoff:", self.ser.xonxoff)
        print("Rts/cts:", self.ser.rtscts)
        print("Dsr/dtr:", self.ser.dsrdtr)
        

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
         
        self.ser.write(b"0") # send a command to the SPS to send a power measurment

        # read a single byte for the length
        resp_len_bytes = self.ser.read()
        
        resp_len = int.from_bytes(resp_len_bytes)

        reply = self.ser.read(resp_len) # read said measurment

        meas_dict = decode_measurement(reply) # decode using protobuf

        voltage_value = meas_dict["data"]["voltage"]
        current_value = meas_dict["data"]["current"]

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
        #reply = reply.decode()
        #reply = reply.strip("\r\n")

        if (reply != b'ok\n'):
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
            self.ser.write(b':OUTP ON\n')
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

            if (v_next <= self.stop):
                return self.set_voltage(v_next)
            else:
                raise StopIteration


        def set_voltage(self, v):
            """Sets the voltage output"""

            self.v = v
            cmd = f":SOUR:VOLT:LEV {v}\n"
            self.ser.write(bytes(cmd, 'ascii'))
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
        self.ser.write(b'*RST\n')
        # Voltage source
        self.ser.write(b':SOUR:FUNC VOLT\n')
        self.ser.write(b':SOUR:VOLT:MODE FIXED\n')
        # 1mA compliance
        self.ser.write(b':SENS:CURR:PROT 10e-3\n')
        # Sensing functions
        self.ser.write(b':SENS:CURR:RANGE:AUTO ON\n')
        self.ser.write(b':SENS:FUNC:OFF:ALL\n')
        self.ser.write(b':SENS:FUNC:ON "VOLT"\n')
        self.ser.write(b':SENS:FUNC:ON "CURR"\n')


    def __del__(self):
        """Destructor

        Turns off output
        """

        self.ser.write(b':OUTP OFF\n')


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

        self.ser.write(b':FORM:ELEM VOLT\n')
        self.ser.write(b':READ?\n')
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

        self.ser.write(b':FORM:ELEM CURR\n') # replace with serial.write with socket.write commands, std library aviable, lots of example code
        self.ser.write(b':READ?\n')
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
            self.sock.sendall(b':OUTP ON\n')
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

            if (v_next <= self.stop):
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
            self.sock.sendall(bytes(cmd, 'ascii'))
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
            self.sock.sendall(b':OUTP ON\n')
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
            if (i_next <= self.stop):
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
            self.sock.sendall(bytes(cmd, 'ascii'))
            return self.i
        


    def __init__(self, host, port, source_mode):
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
        # Reset settings
        self.sock.sendall(b'*RST\n')
        if source_mode == "voltage":
            # Voltage source
            self.sock.sendall(b':SOUR:FUNC VOLT\n')
            self.sock.sendall(b':SOUR:VOLT:MODE FIXED\n')
            # 1mA compliance
            self.sock.sendall(b':SENS:CURR:PROT 10e-3\n')
            # Sensing functions
            self.sock.sendall(b':SENS:CURR:RANGE:AUTO ON\n')
            self.sock.sendall(b':SENS:FUNC:OFF:ALL\n')
            self.sock.sendall(b':SENS:FUNC:ON "VOLT"\n')
            self.sock.sendall(b':SENS:FUNC:ON "CURR"\n')

        elif source_mode == "current":
            # Current source
            self.sock.sendall(b':SOUR:FUNC CURR\n')
            self.sock.sendall(b':SOUR:CURR:MODE FIXED\n')
            # 1V compliance
            self.sock.sendall(b':SENSE:CURR:PROT 1\n')
            # Sensing functions
            self.sock.sendall(b':SENS:VOLT:RANGE:AUTO ON\n')
            self.sock.sendall(b':SENS:FUNC:OFF:ALL\n')
            self.sock.sendall(b':SENS:FUNC:ON "CURR"\n')
            self.sock.sendall(b':SENS:FUNC:ON "VOLT"\n')


    def __del__(self):
        """Destructor

        Turns off output
        """

        self.sock.sendall(b':OUTP OFF\n')


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

        return self.VoltageIterator(self.sock, start, stop, step)
    
    def irange(self, start, stop , step) -> CurrentIterator:
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
        return self.CurrentIterator(self.sock, start, stop, step)


    def get_voltage(self) -> float:
        """Measure voltage supplied to the SPS from SMU

        Returns
        -------
        float
            Measured voltage
        """

        self.sock.sendall(b':FORM:ELEM VOLT\n')
        self.sock.sendall(b':READ?\n')
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

        self.sock.sendall(b':FORM:ELEM CURR\n') # replace with serial.write with socket.write commands, std library aviable, lots of example code
        self.sock.sendall(b':READ?\n')
        # receive response
        reply = self.sock.recv(256)
        # strip trailing \r\n characters
        reply = reply.strip()
        # convert to float and return
        return float(reply)


# if __name__ == "__main__":
#     parser = argparse.ArgumentParser(description="""Automated data recorder
#         for Soil Power Sensor board using serial to control Keithley 2400 SMU""")
    
#     parser.add_argument("--source_mode", type=str, choices=["voltage", "current"], required=True, help="Mode to source either voltage or current")
    
#     range_group = parser.add_argument_group("Range")
#     parser.add_argument("--samples", type=int, default=10, required=False, help="Number of samples to take at each voltage level")
#     parser.add_argument("--startV", type=float, default= -2.0, required=False, help="Start value in voltage")
#     parser.add_argument("--stopV", type=float, default= 2.0, required=False, help="End value in voltage")
#     parser.add_argument("--stepV", type=float, default= 0.1, required=False, help="Step between voltages")

#     parser.add_argument("--startI", type=float, default= -0.0009, required=False, help="Start value in amps")
#     parser.add_argument("--stopI", type=float, default= 0.0009, required=False, help="End value in amps")
#     parser.add_argument("--stepI", type=float, default= 0.0001, required=False, help="Step between amps")
    
#     source_group = parser.add_mutually_exclusive_group(required=True)
#     source_group.add_argument("--smu_port", type=str, help="SMU serial port (if SMU is configured to serial)")
#     source_group.add_argument("--smu_host", type=str, help="SMU host in the format ip:port")
    
#     parser.add_argument("sps_port", type=str, help="SPS serial port")
#     parser.add_argument("data_file", type=str, default="data.csv", help="Path to store data file")

#     args = parser.parse_args() 
     
#     sps = SoilPowerSensorController(args.sps_port) 
#     if args.smu_port:
#         smu = SMUSerialController(args.smu_port, args.source_mode)
#     elif args.smu_host:
#         host, port = args.smu_host.split(":")
#         smu = SMULANController(host, int(port), args.source_mode)

#     data = {
#         "V": [],
#         "I": [],
#         "V_in": [],
#         "I_in": [],
#         "I_sps": [],
#         "V_sps": [],
#     }

#     if args.source_mode == "voltage":
#         iterator = smu.vrange(args.startV, args.stopV, args.stepV)
#     elif args.source_mode == "current":
#         iterator = smu.irange(args.startI, args.stopI, args.stepI)

#     for value in tqdm(iterator):
#         time.sleep(5)
#         for _ in range(args.samples):
#             if args.source_mode == "voltage":
#                 data["V"].append(value)
#                 data["I"].append(0)
#             elif args.source_mode == "current":
#                 data["I"].append(value)
#                 data["V"].append(0)
            
#             measured_voltage, measured_current = sps.get_power()

#             data["I_in"].append(smu.get_current())
#             data["V_in"].append(smu.get_voltage())

#             data["V_sps"].append(measured_voltage)
#             data["I_sps"].append(measured_current)

#     data_df = pd.DataFrame(data)
#     print(data_df)
#     data_df.to_csv(args.data_file, index=False)
