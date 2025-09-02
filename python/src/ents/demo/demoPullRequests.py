"""
File: demoPullRequest.py
Author: Caden Grace Jacobs

This python script pulls data from the DirtViz API from various sensors
to preform measurements and calculations. 
"""

import requests
from datetime import datetime, timezone, timedelta
import pandas as pd
from tabulate import tabulate
import pytz 


class DirtVizClient:
    BASE_URL = "https://dirtviz.jlab.ucsc.edu/api/"

    def __init__(self):
        self.session = requests.Session()

    def get_sensor_data(self, name, measurement, cellId, start=None, end=None):
        """Get sensor data for a specific cell"""

        endpoint = f"sensor/?name={name}&measurement={measurement}&cellId={cellId}"
        params = {}

        if start and end:
            # Convert to UTC for the API call
            start_utc = start.astimezone(pytz.UTC)
            end_utc = end.astimezone(pytz.UTC)
            params = {
                "startTime": start_utc.strftime("%a, %d %b %Y %H:%M:%S GMT"),
                "endTime": end_utc.strftime("%a, %d %b %Y %H:%M:%S GMT"),
            }

        response = self.session.get(f"{self.BASE_URL}{endpoint}", params=params)
        response.raise_for_status()
        return response.json()

def format_data_display(df, cell_id, measurement_type):
    """Format the data output with timestamp as first column"""
    ca_tz = pytz.timezone('America/Los_Angeles')

    # Ensure timestamp exists and is first column
    if "timestamp" in df.columns:
        cols = ["timestamp"] + [col for col in df.columns if col != "timestamp"]
        df = df[cols]

        # Format timestamp nicely
        df["timestamp"] = pd.to_datetime(df["timestamp"]).dt.tz_localize('UTC').dt.tz_convert(ca_tz)
        df["timestamp"] = df["timestamp"].dt.strftime("%m-%d-%Y %H:%M:%S")

    # Calculate statistics - check if 'data' column exists
    data_values = df['data'] if 'data' in df.columns else pd.Series(dtype=float)
    
    # Determine the unit and measurement name based on the measurement type
    if measurement_type == "pressure":
        unit = "kPa"
        measurement_name = "Pressure"
        column_name = f"{measurement_name} ({unit})"
    elif measurement_type == "humidity":
        unit = "%"
        measurement_name = "Humidity"
        column_name = f"{measurement_name} ({unit})"
    elif measurement_type == "flow rate":
        unit = "L/min"
        measurement_name = "Flow Rate"
        column_name = f"{measurement_name} ({unit})"
    else:
        unit = df['unit'].iloc[0] if 'unit' in df.columns and len(df) > 0 else "units"
        measurement_name = measurement_type.capitalize()
        column_name = f"{measurement_name} ({unit})"
    
    stats = {
        "Cell ID": cell_id,
        "Measurement Type": measurement_name,
        "Time Range": (
            f"{df['timestamp'].iloc[0]} to {df['timestamp'].iloc[-1]}"
            if len(df) > 0
            else "N/A"
        ),
        "Data Points": len(df),
        f"Avg {column_name}": f"{data_values.mean():.2f}" if not data_values.empty else "N/A",
        f"Max {column_name}": f"{data_values.max():.2f}" if not data_values.empty else "N/A",
    }

    column_rename = {
        "timestamp": "Measurement Time (PT)",
        "data": column_name,
    }
    # Apply renaming
    df = df.rename(columns=column_rename)

    # Display header
    print("\n" + "=" * 70)
    print(f"CELL {cell_id} {measurement_name.upper()} DATA SUMMARY".center(70))
    for key, value in stats.items():
        print(f"• {key:<20}: {value}")  # Display the summary information
    print("=" * 70 + "\n")

    # Display sample data with timestamp first
    if len(df) > 0:
        print("DATA BY TIMESTAMPS:")
        # Select only the columns we want to display
        display_columns = ["Measurement Time (PT)", column_name]
        display_df = df[display_columns]
        print(
            tabulate(
                display_df,
                headers="keys",
                tablefmt="grid",
                stralign="center",
                showindex=False,
                numalign="center",
                floatfmt=".3f",  # Format numbers to 3 decimal places
            )
        )
    else:
        print("No data available to display")

    print("\n" + "=" * 80)

def display_menu():
    """Display the menu of available cells and measurements"""
    print("\n" + "=" * 50)
    print("DIRT VIZ SENSOR DATA VIEWER".center(50))
    print("=" * 50)
    print("Available Cells and Measurements:")
    print("1. Cell 1350 - Pressure (kPa)")
    print("2. Cell 1448 - Humidity (%)")
    print("3. Cell 1353 - Flow Rate (L/min)")
    print("4. Custom Cell (enter cell ID and measurement type)")
    print("5. Exit")
    print("=" * 50)

def get_cell_info(choice):
    """Return cell information based on user choice"""
    cell_info = {
        1: {"cell_id": 1350, "sensor_name": "sen0257", "measurement": "pressure"},
        2: {"cell_id": 1448, "sensor_name": "sen03808", "measurement": "humidity"},
        3: {"cell_id": 1353, "sensor_name": "yfs210c", "measurement": "flow"},
    }
    return cell_info.get(choice, None)

def display_time_range_menu():
    """Display the time range selection menu"""
    print("\n" + "=" * 40)
    print("SELECT TIME RANGE".center(40))
    print("=" * 40)
    print("1. Today's data")
    print("2. This week's data")
    print("3. Custom date range")
    print("=" * 40)

def get_valid_date(prompt):
    """Prompt user for a valid date and keep asking until valid input is provided"""

    ca_tz = pytz.timezone('America/Los_Angeles')

    while True:
        date_input = input(prompt)
        if not date_input:  # Use default if empty input
            return datetime.now(ca_tz).replace(hour=0, minute=0, second=0, microsecond=0)
        
        try:
            # Validate date format
            year, month, day = map(int, date_input.split('-'))
            # Validate date values
            if year < 2000 or year > 2100 or month < 1 or month > 12 or day < 1 or day > 31:
                print("Invalid date values. Please enter a valid date between 2000-2100.")
                continue
                
            return ca_tz.localize(datetime(year, month, day, 0, 0, 0))
        except ValueError:
            print("Invalid date format. Please use YYYY-MM-DD format (e.g., 2025-08-21).")

def get_time_range():
    """Get the time range based on user selection"""
    display_time_range_menu()
    ca_tz = pytz.timezone('America/Los_Angeles')

    while True:
        
        try:
            time_choice = int(input("\nEnter your choice (1-3): "))
            
            now_ca = datetime.now(ca_tz)
            
            if time_choice == 1:
                # Today's data
                start = now_ca.replace(hour=0, minute=0, second=0, microsecond=0)
                return start, now_ca
                
            elif time_choice == 2:
                # This week's data (last 7 days)
                start = now_ca.replace(hour=0, minute=0, second=0, microsecond=0) - timedelta(days=7)
                return start, now_ca
                
            elif time_choice == 3:
                # Custom date range
                start = get_valid_date("Enter start date (YYYY-MM-DD): ")
                end = get_valid_date("Enter end date (YYYY-MM-DD): ")
                # Set time to end of day for end date
                end = end.replace(hour=23, minute=59, second=59)
                return start, end
                
            else:
                print("Invalid choice. Please select 1-3.")
                
        except ValueError:
            print("Please enter a valid number.")

if __name__ == "__main__":
    client = DirtVizClient()
    
    while True:
        display_menu()
        try:
            choice = int(input("\nEnter your choice (1-5): "))
            
            if choice == 5:
                print("Exiting program. Goodbye!")
                break
                
            if choice == 4:
                # Custom cell
                cell_id = int(input("Enter cell ID: "))
                sensor_name = input("Enter sensor name (e.g., sen0257, yfs210c, etc.): ")
                measurement = input("Enter measurement type (e.g., pressure, humidity): ")
            else:
                cell_info = get_cell_info(choice)
                if not cell_info:
                    print("Invalid choice. Please try again.")
                    continue
                
                cell_id = cell_info["cell_id"]
                sensor_name = cell_info["sensor_name"]
                measurement = cell_info["measurement"]
            
             # Get time range
            print(f"\nFetching {measurement} data for cell {cell_id}...")
            start, end = get_time_range()
            
            # Fetch and display data
            data = client.get_sensor_data(sensor_name, measurement, cell_id, start, end)
            
            if data:
                df = pd.DataFrame(data)
                format_data_display(df, cell_id, measurement)
            else:
                print("No data received for the specified time range.")
                
        except ValueError:
            print("Please enter a valid number.")
        except requests.exceptions.HTTPError as e:
            print(f"\nHTTP Error: {e}")
            print(f"Response: {e.response.text[:500]}...")
        except Exception as e:
            print(f"\nUnexpected error: {str(e)}")
        
        # Ask if user wants to continue
        continue_choice = input("\nWould you like to view another cell? (y/n): ").lower()
        if continue_choice != 'y':
            print("Exiting program. Goodbye!")
            break