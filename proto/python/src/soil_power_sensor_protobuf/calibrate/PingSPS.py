"""To test connection to Soil Power Sensor over serial

Stephen Taylor 5/20/2024
"""

import serial


class SerialPing:
    """Simple serial ping utility"""

    def __init__(self, port, baudrate=115200, xonxoff=True):
        """Constructor

        Initializes serial connection.

        Parameters
        ----------
        port : str
            Serial port name (e.g., COM1, /dev/ttyUSB0)
        baudrate : int, optional
            Baud rate for serial communication (default is 115200, STM32 functions at 115200)
        xonxoff  : bool, optional
            Flow control (default is on)
        """
        self.ser = serial.Serial(port, baudrate=baudrate, xonxoff=xonxoff, timeout=1)
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

    def ping(self):
        """Ping the device and return the response"""
        # pdb.set_trace()
        self.ser.write(b"check\n")  # Send ping command
        response = self.ser.readline()  # Read response
        return response

    def close(self):
        """Close serial connection"""
        self.ser.close()


# Example usage
if __name__ == "__main__":
    # Replace 'COM1' with the appropriate serial port on your system
    port = "COM14"

    # Create SerialPing object
    serial_ping = SerialPing(port)

    try:
        # Ping the device
        response = serial_ping.ping()
        print("Response:", response)

    finally:
        # Close serial connection
        serial_ping.close()
