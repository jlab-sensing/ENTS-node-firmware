'''
remote_solenoid.py
Author: Caden Jacobs
Date: 08/19/2025

This python script connectes to the IP configured from the
ESP32 to remotely controll the solenoid valve for an irrigation system.

The solenoid can be opened indefinetly, closed indefinetly, or opened
for a set duration of time.
'''

import requests
import sys

ENTS_IP = "172.31.100.193"  # Default ENTS WiFi with HARE_Lab
PORT = 80   #Server Port

def send_command(command):

    if command == "open":

        #Error Cases
        if (len(sys.argv) > 2):
            print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py open\r\n")
            sys.exit(1)
    
        response = requests.post(
        f"http://{ENTS_IP}:{PORT}/{command}", timeout=5)

        print("Triggering solenoid")
        print(f"Response: {response.text}")

    elif command == "close":

        #Error Cases
        if (len(sys.argv) > 2):
            print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py close\r\n")
            sys.exit(1)

        response = requests.post(
        f"http://{ENTS_IP}:{PORT}/{command}",timeout=5)

        print("Triggering solenoid")
        print(f"Response: {response.text}")

    elif command == "timed":

        #Error Cases
        if (len(sys.argv) < 3):
            print("Error: Time duration not set.\r\nFormat: python remote_solenoid.py timed [seconds].\r\n")
            sys.exit(1)

        elif (len(sys.argv) > 3):
            print("Error: Too many arguments.\r\nFormat: python remote_solenoid.py timed [seconds].\r\n")
            sys.exit(1)

        #Get the time from command line
        setTime={"time":sys.argv[2]}

        response = requests.post(
        f"http://{ENTS_IP}:{PORT}/{command}", params=setTime, timeout=5)

        print(f"Turning on solenoid for {sys.argv[2]} seconds.")
        print(f"Response: {response.text}")

    elif command == "state":
        # Error Cases
        if len(sys.argv) > 2:
            print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py state\r\n")
            sys.exit(1)

        response = requests.get(
            f"http://{ENTS_IP}:{PORT}/{command}", timeout=5)
        
        print(f"Current state: {response.text}")
    
    else:
        print(f"Error: Unknown command '{command}'")
        #print_usage()
        sys.exit(1)

    # Check response status
    if response.status_code != 200:
        print(f"Error: Server responded with status {response.status_code}")
        print(f"Response: {response.text}")
    print(f"Response: {response.text}")
   
   #except Exception as e:
       # print(f"Error: {str(e)}")

if __name__ == "__main__":

    #Error if syntax not correct
    if (len(sys.argv) < 2):
        options = ["open", "close", "timed [seconds]", "state"]
        message = "\n".join([
            "Error: No solenoid command received.",
            "Format: python remote_solenoid.py [command].",
            "Please type one of the following options:",
            *[f"   {option}" for option in options]
        ])
        print("\n\n"+message)
        sys.exit(1)

    #Send the command
    send_command(sys.argv[1])