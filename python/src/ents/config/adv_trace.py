"""
@brief testing advance trace receive in stm32
@file adv_trace.py
@author Ahmed Hassan Falah
@date 2024-10-12
"""

import serial
import serial.tools.list_ports


def sendToUART():
    """
    Sends the encoded configuration data via UART.
    """
    ser = None
    try:
        # Open the serial port
        ser = serial.Serial(port="COM6", baudrate=115200, timeout=2)
        # Send data (value 1)
        data = bytes([1])
        print(f"Sending: {data}")
        ser.write(data)
        print(
            "________________________________________________________________________"
        )
        print(f"{data}")

        # Read acknowledgment (1 byte)
        ack = ser.read(1)
        print(f"Received from STM32: {ack}")
        print(
            "________________________________________________________________________"
        )

        # Check acknowledgment
        if ack == b"A":
            print("Success")
        elif ack == b"N":
            print("Error NACK")
            return False
        else:
            print("Error")
            return False

    except serial.SerialException as e:
        print(f"UART Error: {e}")
        return False

    finally:
        if ser is not None:
            ser.close()


if __name__ == "__main__":
    sendToUART()
