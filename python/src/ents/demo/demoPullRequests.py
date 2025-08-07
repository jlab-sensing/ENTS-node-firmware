import requests
from datetime import datetime
import pandas as pd

# === CONFIG ===
BASE_URL = "https://dirtviz.jlab.ucsc.edu/api"  # Update if needed
CELL_ID = "Cell-A1"  # Replace with wanted cell ID
FIELDS = ["temperature", "humidity", "v", "p", "i"]  # Choose your desired fields
START = "2025-02-16T00:00:00Z"  # ISO 8601 format #Change to 2 weeks ago?
END = "2025-02-17T00:00:00Z"    #Update to most recent time

def fetch_cell_data(cell_id: str, start: str, end: str, fields: list[str]):
    url = f"{BASE_URL}/cells/{cell_id}/data"
    params = {
        "start": start,
        "end": end,
        "fields": ",".join(fields),
    }

    try:
        response = requests.get(url, params=params)
        response.raise_for_status()
        return response.json()

    except requests.RequestException as e:
        print("Error during request:", e)
        return None

def print_cell_data(data: list[dict]):
    if not data:
        print("No data received.")
        return

    df = pd.DataFrame(data)
    print(df.to_string(index=False))

if __name__ == "__main__":
    data = fetch_cell_data(CELL_ID, START, END, FIELDS)
    print_cell_data(data)
