"""To test connection via socket to Kiethley 2450

Stephen Taylor, 5/20/2024
"""

import socket


def ping_smu(host, port):
    try:
        # Create a TCP/IP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Set a timeout for the connection attempt
        sock.settimeout(1)

        # Connect to the SMU
        sock.connect((host, port))

        # Send the *IDN? command
        sock.sendall(b"*RST\n")
        sock.sendall(b"*IDN?\n")

        # Receive the response
        response = sock.recv(1024)
        response = response.strip()
        print("Response from SMU:", response)

        # Close the socket
        sock.close()

        # Return True if connection successful
        return True
    except Exception as e:
        # Connection failed
        print(f"Error: {e}")
        return False


if __name__ == "__main__":
    # Specify the IP address or hostname of the SMU and its port
    smu_host = (
        "128.114.204.56"  # Replace with the actual IP address or hostname of the SMU
    )
    smu_port = 23  # Replace with the actual port used by the SMU

    # Ping the SMU
    if ping_smu(smu_host, smu_port):
        print("SMU is reachable.")
    else:
        print("Failed to ping SMU.")
