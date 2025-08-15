import requests
from datetime import datetime, timezone
import pandas as pd
from tabulate import tabulate

class DirtVizClient:
    BASE_URL = "https://dirtviz.jlab.ucsc.edu/api/"
    
    def __init__(self):
        self.session = requests.Session()

    def cell_from_name(self, name, start=None, end=None):
         """Get power data for a specific cell"""

        
    def get_power_data(self, cell_id, start=None, end=None):
        """Get power data for a specific cell"""
        endpoint = f"power/{cell_id}"
        params = {}
        
        if start and end:
            params = {
                "startTime": start.strftime("%a, %d %b %Y %H:%M:%S GMT"),
                "endTime": end.strftime("%a, %d %b %Y %H:%M:%S GMT")
            }
        
        response = self.session.get(f"{self.BASE_URL}{endpoint}", params=params)
        response.raise_for_status()
        return response.json()

def format_data_display(df, cell_id):
    """Format the data output with timestamp as first column"""

    # Ensure timestamp exists and is first column
    if 'timestamp' in df.columns:
        cols = ['timestamp'] + [col for col in df.columns if col != 'timestamp']
        df = df[cols]
        
        # Format timestamp nicely
        df['timestamp'] = pd.to_datetime(df['timestamp'])
        df['timestamp'] = df['timestamp'].dt.strftime('%m-%d-%Y %H:%M:%S')
    
    # Calculate statistics
    stats = {
        "Cell ID": cell_id,
        "Time Range": f"{df['timestamp'].iloc[0]} to {df['timestamp'].iloc[-1]}" if len(df) > 0 else "N/A",
        "Data Points": len(df),
        "Avg Voltage (mV)": f"{df['v'].mean():.2f}" if 'v' in df.columns else "N/A",
        "Max Voltage (mV)": f"{df['v'].max():.2f}" if 'v' in df.columns else "N/A",
        "Avg Current (µA)": f"{df['i'].mean():.2f}" if 'i' in df.columns else "N/A", 
        "Avg Power (µW)": f"{df['p'].mean():.2f}" if 'p' in df.columns else "N/A"
    }

    column_rename = {
        'timestamp': 'Measurement Times',
        'v': 'Voltage (mV)',
        'i': 'Current (µA)',
        'p': 'Power (µW)'
    }
    # Apply renaming
    df = df.rename(columns=column_rename)
    
    # Display header
    print("\n" + "="*60)
    print(f"📊 CELL {cell_id} POWER DATA SUMMARY".center(60))
    for key, value in stats.items():
        print(f"• {key:<20}: {value}") #Display the summary information
    print("="*60 + "\n")
    
    # Display sample data with timestamp first
    if len(df) > 0:
        print("🔍 DATA BY TIMESTAMPS:")
        print(tabulate(
            df,
            headers='keys',
            tablefmt='grid',  # Changed to grid for better column alignment
            stralign='center',  # Right-align numbers
            showindex=False,
            numalign="center"
        ))
    else:
        print("⚠️ No data available to display")
    
    print("\n" + "="*80)

if __name__ == "__main__":
    client = DirtVizClient()
    
    try:
        cell_id = 893 #Figure out how to do by name on DirtViz
        start = datetime(2025, 8, 12, tzinfo=timezone.utc)
        end = datetime.now(timezone.utc)
        
        print(f"\n🔌 Fetching power data for cell {cell_id}...")
        data = client.get_power_data(cell_id, start, end)
        
        if data:
            df = pd.DataFrame(data)
            format_data_display(df, cell_id)
            
            # Save to CSV with timestamp first
            # df.to_csv(f"cell_{cell_id}_power_data.csv", index=False)
            #print(f"💾 Data saved to cell_{cell_id}_power_data.csv")
        else:
            print("⚠️ No data received for the specified time range.")
            
    except requests.exceptions.HTTPError as e:
        print(f"\n❌ HTTP Error: {e}")
        print(f"💡 Response: {e.response.text[:500]}...")
    except Exception as e:
        print(f"\n⚠️ Unexpected error: {str(e)}")