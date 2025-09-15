import requests
import sys
import json
import argparse
import threading
import time

ENTS_IP = "172.31.105.241"  # Default ENTS WiFi with HARE_Lab
PORT = 80   #Server Port
BASE_URL = f"http://{ENTS_IP}:{PORT}"

# Global variables for auto irrigation monitoring
auto_monitoring_active = False
monitoring_thread = None
    
def get_system_status():
    """Get current system status from ESP32"""
    try:
        response = requests.get(f"{BASE_URL}/status", timeout=5)
        if response.status_code == 200:
            return response.json()
    except:
        pass
    return None

def print_status_summary(status):
    """Print a clean status summary"""
    print("\n" + "="*60)
    print("SYSTEM STATUS SUMMARY")
    print("="*60)
    
    if status:
        print(f"Current Soil Humidity: {status.get('current_moisture', 'N/A')}%")
        print(f"Solenoid State: {status.get('solenoid_state', 'unknown').upper()}")
        print(f"Auto Irrigation: {'ENABLED' if status.get('auto_irrigation_enabled', False) else 'DISABLED'}")
        print(f"Irrigation Thresholds: Below {status.get('min_threshold', 'N/A')}% & Above {status.get('max_threshold', 'N/A')}%")
        print(f"Check Interval: {status.get('check_interval_seconds', 'N/A')} seconds")
    else:
        print("Could not fetch system status")
    
    print(f"Monitoring Active: {'YES' if auto_monitoring_active else 'NO'}")
    print("="*60)

def monitor_status_continuously(check_interval=30):
    """Continuously monitor system status from ESP32"""
    global auto_monitoring_active
    
    print(f"Starting continuous status monitoring (interval: {check_interval}s)")
    cycle_count = 0
    status_print_interval = 5  # Print status every 5 cycles
    
    while auto_monitoring_active:
        cycle_count += 1
        
        try:
            # Get system status from ESP32
            status = get_system_status()
            
            if status:
                # Print status summary every few cycles
                if cycle_count % status_print_interval == 0:
                    print_status_summary(status)
                else:
                    print(f"Cycle {cycle_count}: Moisture={status.get('current_moisture', 'N/A')}%, Solenoid={status.get('solenoid_state', 'unknown').upper()}")
            else:
                print(f"Cycle {cycle_count}: Could not fetch system status")
                
        except Exception as e:
            print(f"Cycle {cycle_count}: Error - {e}")
        
        # Wait for next check with interruptible sleep
        for i in range(check_interval):
            if not auto_monitoring_active:
                break
            time.sleep(1)
    
    print("Status monitoring stopped")

def start_auto_monitoring(check_interval=30):
    """Start the auto monitoring thread"""
    global auto_monitoring_active, monitoring_thread
    
    if monitoring_thread and monitoring_thread.is_alive():
        print("Auto monitoring is already running")
        return
    
    auto_monitoring_active = True
    monitoring_thread = threading.Thread(
        target=monitor_status_continuously, 
        args=(check_interval,),
        daemon=True
    )
    monitoring_thread.start()
    print("Auto monitoring started")

def stop_auto_monitoring():
    """Stop the auto monitoring thread"""
    global auto_monitoring_active
    
    auto_monitoring_active = False
    print("Auto monitoring stopping...")
    # Thread will exit on next loop iteration

def send_command(command):
    global auto_monitoring_active
    
    try:
        response = None

        if command == "open":
            if (len(sys.argv) > 2):
                print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py open\r\n")
                sys.exit(1)
        
            response = requests.post(f"{BASE_URL}/open", timeout=5)
            print("Triggering solenoid")
            print(f"Response: {response.text}")

        elif command == "close":
            if (len(sys.argv) > 2):
                print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py close\r\n")
                sys.exit(1)

            response = requests.post(f"{BASE_URL}/close", timeout=5)
            print("Triggering solenoid")
            print(f"Response: {response.text}")

        elif command == "timed":
            if (len(sys.argv) < 3):
                print("Error: Time duration not set.\r\nFormat: python remote_solenoid.py timed [seconds].\r\n")
                sys.exit(1)
            elif (len(sys.argv) > 3):
                print("Error: Too many arguments.\r\nFormat: python remote_solenoid.py timed [seconds].\r\n")
                sys.exit(1)

            setTime = {"time": sys.argv[2]}
            response = requests.post(f"{BASE_URL}/timed", params=setTime, timeout=5)
            print(f"Turning on solenoid for {sys.argv[2]} seconds.")
            print(f"Response: {response.text}")

        elif command == "state":
            if len(sys.argv) > 2:
                print("\r\nError: Too many arguments.\r\nFormat: python remote_solenoid.py state\r\n")
                sys.exit(1)

            response = requests.get(f"{BASE_URL}/state", timeout=5)
            print(f"Current state: {response.text}")

        elif command == "set_thresholds":
            # Set thresholds via API
            if len(sys.argv) != 4:
                print("Error: Usage: python remote_solenoid.py set_thresholds <min> <max>")
                print("Example: python remote_solenoid.py set_thresholds 30 60")
                sys.exit(1)
            
            min_thresh = sys.argv[2]
            max_thresh = sys.argv[3]
            params = {"min": min_thresh, "max": max_thresh}
            
            response = requests.post(f"{BASE_URL}/irrigation_setup", data=params, timeout=5)
            print(f"Set thresholds: Min={min_thresh}%, Max={max_thresh}%")
            print(f"Response: {response.text}")
        
        elif command == "auto_irrigation":
            # One-step command to enable auto irrigation with thresholds
            if len(sys.argv) != 4:
                print("Error: Usage: python remote_solenoid.py auto_irrigation <min> <max>")
                print("Example: python remote_solenoid.py auto_irrigation 30 60")
                sys.exit(1)
            
            min_thresh = sys.argv[2]
            max_thresh = sys.argv[3]
            params = {"min": min_thresh, "max": max_thresh, "enable": "true"}
            
            response = requests.post(f"{BASE_URL}/irrigation_setup", data=params, timeout=5)
            print(f"Auto irrigation enabled: Moisture < {min_thresh}% → OPEN, > {max_thresh}% → CLOSE")
            print(f"Response: {response.text}")
            
            # Start continuous monitoring
            start_auto_monitoring()
    
        elif command == "auto_on":
            # Enable automatic irrigation
            response = requests.post(f"{BASE_URL}/auto", data={"enable": "true"}, timeout=5)
            print("Automatic irrigation enabled")
            print(f"Response: {response.text}")
            
            # Print initial status
            status = get_system_status()
            print_status_summary(status)
            
            # Start continuous monitoring
            start_auto_monitoring()

        elif command == "auto_off":
            # Disable automatic irrigation
            response = requests.post(f"{BASE_URL}/auto", data={"enable": "false"}, timeout=5)
            print("Automatic irrigation disabled")
            print(f"Response: {response.text}")
            
            # Stop continuous monitoring
            stop_auto_monitoring()

        elif command == "status":
            # Get complete system status
            status = get_system_status()
            if status:
                print("System Status:")
                print(f"  Solenoid State: {status.get('solenoid_state', 'unknown')}")
                print(f"  Auto Irrigation: {'Enabled' if status.get('auto_irrigation_enabled', False) else 'Disabled'}")
                print(f"  Irrigation Thresholds: {status.get('min_threshold', 'N/A')}% - {status.get('max_threshold', 'N/A')}%")
                print(f"  Check Interval: {status.get('check_interval_seconds', 'N/A')} seconds")  # Changed to seconds
                print(f"  Current Moisture: {status.get('current_moisture', 'N/A')}%")
                print(f"  Monitoring Active: {'Yes' if auto_monitoring_active else 'No'}")
            else:
                print("Could not fetch system status")
        
        elif command == "moisture_check":
            # Manual moisture check - now just displays current moisture from ESP32
            status = get_system_status()
            if status and 'current_moisture' in status:
                moisture = status['current_moisture']
                print(f"Current moisture from ESP32: {moisture}%")
                print_status_summary(status)
            else:
                print("Could not fetch moisture reading from ESP32")

        else:
            print(f"Error: Unknown command '{command}'")
            print_usage()
            sys.exit(1)

        # Check response status only for commands that made HTTP requests
        if response is not None:
            if response.status_code != 200:
                print(f"Error: Server responded with status {response.status_code}")
                print(f"Response: {response.text}")
            else:
                print(f"Response: {response.text}")
            
    except requests.exceptions.RequestException as e:
        print(f"Error: Could not connect to ESP32 at {ENTS_IP}")
        print(f"Details: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def print_usage():
    """Display usage instructions"""
    message = "\n".join([
        "\nAvailable Commands:",
        "  Basic Control:",
        "    open                 - Open solenoid indefinitely",
        "    close                - Close solenoid",
        "    timed [seconds]      - Open for specified duration", 
        "    state                - Check current solenoid state",
        "",
        "  Automatic Irrigation:",
        "    auto_irrigation <min> <max> - Enable auto mode with thresholds + start monitoring",
        "    auto_on              - Enable automatic irrigation + start monitoring",
        "    auto_off             - Disable automatic irrigation + stop monitoring",
        "    set_thresholds <min> <max> - Set thresholds only",
        "    status               - Show complete system status",
        "    moisture_check       - Check current moisture reading from ESP32",
        "",
        "Examples:",
        "  python remote_solenoid.py timed 300          # Open for 5 minutes",
        "  python remote_solenoid.py auto_irrigation 50 75  # Auto mode: <50% open, >75% close",
        "  python remote_solenoid.py set_thresholds 25 55  # Update thresholds only",
        "  python remote_solenoid.py auto_on            # Enable auto irrigation",
        "  python remote_solenoid.py status             # Show system status",
        "  python remote_solenoid.py moisture_check     # Check current moisture from ESP32"
    ])
    print(message)

def cleanup():
    """Cleanup function to stop monitoring on exit"""
    stop_auto_monitoring()

if __name__ == "__main__":
    # Register cleanup function
    import atexit
    atexit.register(cleanup)
    
    if (len(sys.argv) < 2):
        print("Error: No solenoid command received.")
        print_usage()
        sys.exit(1)

    # Send the command
    send_command(sys.argv[1])

    # Keep main thread alive if auto monitoring is started
    if auto_monitoring_active:
        print("\nAuto monitoring running. Press Ctrl+C to stop.")
        try:
            while auto_monitoring_active:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nStopping auto monitoring...")
            stop_auto_monitoring()