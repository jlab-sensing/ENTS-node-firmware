import argparse
from datetime import datetime

from .encode import encode_power_measurement, encode_phytos31_measurement, encode_teros12_measurement, encode_response
from .decode import decode_measurement, decode_response
from .esp32 import encode_esp32command, decode_esp32command

def entry():
    """Command line interface entry point"""
    
    parser = argparse.ArgumentParser()
    
    subparsers = parser.add_subparsers()
    
    encode_parser = create_encode_parser(subparsers)
    decode_parser = create_decode_parser(subparsers)
    
    args = parser.parse_args()
    args.func(args)
    
def create_encode_parser(subparsers):
    """Create encode command subparser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """
    
    encode_parser = subparsers.add_parser('encode', help='Encode data')
   
    print_format = encode_parser.add_mutually_exclusive_group() 
    print_format.add_argument('--hex', action='store_true', help='Print as hex values (default)')
    print_format.add_argument('--raw', action='store_true', help='Print raw bytes object')
    print_format.add_argument('--c', action='store_true', help='Print bytes for copying to c')

    encode_subparsers = encode_parser.add_subparsers(title='Message type', dest='type') 
    
    def create_measurement_parser(encode_subparsers): 
        measurement_parser = encode_subparsers.add_parser('measurement', help='Proto "Measurement" message')
        measurement_subparser = measurement_parser.add_subparsers(title='Measurement type')
        
        # metadata
        measurement_parser.add_argument('ts', type=int, help='Unix epoch timestamp')
        measurement_parser.add_argument('cell', type=int, help='Cell Id')
        measurement_parser.add_argument('logger', type=int, help='Logger Id')
        
        power_parser = measurement_subparser.add_parser('power', help='PowerMeasurement')
        power_parser.add_argument('voltage', type=float, help="Voltage in (V)") 
        power_parser.add_argument('current', type=float, help="Current in (A)")
        power_parser.set_defaults(func=handle_encode_measurement_power)
        
        teros12_parser = measurement_subparser.add_parser('teros12', help='Teros12Measurement')
        teros12_parser.add_argument('vwc_raw', type=float, help='Raw vwc')
        teros12_parser.add_argument('vwc_adj', type=float, help='Calibrated vwc')
        teros12_parser.add_argument('temp', type=float, help='Temperature in C')
        teros12_parser.add_argument('ec', type=int, help='Electrical conductivity')
        teros12_parser.set_defaults(func=handle_encode_measurement_teros12)
        
        phytos31_parser = measurement_subparser.add_parser('phytos31', help='Phytos31Measurement')
        phytos31_parser.add_argument('voltage', type=float, help='Raw voltage (V)')
        phytos31_parser.add_argument('leaf_wetness', type=float, help='Leaf wetness')
        phytos31_parser.set_defaults(func=handle_encode_measurement_phytos31)
        
        return measurement_parser
   
    def create_response_parser(encode_subparsers): 
        response_parser = encode_subparsers.add_parser('response', help='Proto "Response" message')
        response_parser.add_argument('status', type=str, help="Status")
        response_parser.set_defaults(func=handle_encode_response)
        
        return response_parser
   
    def create_esp32command_parser(encode_subparsers): 
        esp32command_parser = encode_subparsers.add_parser('esp32command', help='Proto "Esp32Command" message')
        esp32command_subparser = esp32command_parser.add_subparsers(title='type', help='PageCommand')
        
        test_parser = esp32command_subparser.add_parser('test', help='TestCommand')
        test_parser.add_argument('state', type=str, help='State to put module into')
        test_parser.add_argument('data', type=int, help='Data associated with command')
        test_parser.set_defaults(func=handle_encode_esp32command_test)
        
        esp32_parser = esp32command_subparser.add_parser('page', help='PageCommand')
        esp32_parser.add_argument('type', type=str, help='Request type')
        esp32_parser.add_argument('fd', type=int, help='File descriptor')
        esp32_parser.add_argument('bs', type=int, help='Block size')
        esp32_parser.add_argument('num', type=int, help='Number of bytes')
        esp32_parser.set_defaults(func=handle_encode_esp32command_page)
        
        wifi_parser = esp32command_subparser.add_parser('wifi', help='WiFiCommand')
        wifi_parser.add_argument('type', type=str, help="WiFi command type")
        wifi_parser.add_argument('--ssid', type=str, default="", help="WiFi SSID")
        wifi_parser.add_argument('--passwd', type=str, default="", help="WiFi password")
        wifi_parser.add_argument('--url', type=str, default="", help="Endpoint url")
        wifi_parser.add_argument('--port', type=int, default=0, help="Endpoint port")
        wifi_parser.add_argument('--rc', type=int, default=0, help="Return code")
        wifi_parser.add_argument('--ts', type=str, help="Timestamp")
        wifi_parser.add_argument('--resp', type=str, default=b"", help="Serialized response message")
        wifi_parser.set_defaults(func=handle_encode_esp32command_wifi)
        
        return esp32command_parser
   
    # create subparsers
    create_measurement_parser(encode_subparsers)
    create_response_parser(encode_subparsers)
    create_esp32command_parser(encode_subparsers)

    return encode_parser


def handle_encode_measurement_power(args):
    data = encode_power_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        voltage=args.voltage,
        current=args.current     
    )
    
    print_data(args, data) 
    
def handle_encode_measurement_teros12(args):
    data = encode_teros12_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        vwc_raw=args.vwc_raw,
        vwc_adj=args.vwc_adj,
        temp=args.temp,
        ec=args.ec,
    )
    
    print_data(args, data)

def handle_encode_measurement_phytos31(args):
    data = encode_phytos31_measurement(
        ts=args.ts,
        cell_id=args.cell,
        logger_id=args.logger,
        voltage=args.voltage,
        leaf_wetness=args.leaf_wetness,
    )
    
    print_data(args, data)
    
def handle_encode_response(args):
    if args.status == "SUCCESS":
        success = True,
    elif args.status == "ERROR":
        success = False
    else:
        raise NotImplementedError(f'Response status "{args.status}" not implemented')
    
    data = encode_response(args.status)
    print_data(args, data)

def handle_encode_esp32command_test(args):
    data = encode_esp32command("test", state=args.state, data=args.data)
    print_data(args, data)

def handle_encode_esp32command_page(args):
    data = encode_esp32command(
        "page",
        req=args.type.lower(),
        fd=args.fd,
        bs=args.bs,
        n=args.num
    )
    print_data(args, data)
    
def handle_encode_esp32command_wifi(args):
    # process timestamp string
    if (args.ts):
        ts = datetime.fromisoformat(args.ts)
        ts = ts.timestamp()
    else:
        ts = 0
    
    data = encode_esp32command(
        "wifi",
        _type=args.type.lower(),
        ssid=args.ssid,
        passwd=args.passwd,
        url=args.url,
        port=args.port,
        rc=args.rc,
        ts=ts,
        resp=args.resp
    )
    print_data(args, data)
 
def print_data(args, data: bytes) -> str:
    if (args.c):
        print_bytes_c(data)
    elif (args.raw):
        print(data)
    else:
        print(data.hex())

def print_bytes_c(data : bytes) -> str:
    """Formats serialized data into c bytes array"""

    # format data string
    data_str = "uint8_t data[] = {"
    hex_str = [hex(d) for d in data]
    data_str += ", ".join(hex_str) 
    data_str += "};"

    # print data string
    print(data_str)
    
    # print length of data string
    print(f'size_t data_len = {len(hex_str)};')

def create_decode_parser(subparsers):
    """Create decode command parser

    Args:
        subparsers: Reference to subparser group

    Returns:
        Reference to new subparser
    """
    
    decode_parser = subparsers.add_parser('decode', help='Decode data')
    
    decode_subparsers = decode_parser.add_subparsers(title='Message type', dest='type')
   
    # measurement
    measurement_parser = decode_subparsers.add_parser('measurement', help='Proto "Measurement" message')
    measurement_parser.set_defaults(func=handle_decode_measurement)
    
    # response
    response_parser = decode_subparsers.add_parser('response', help='Proto "Response" message')
    response_parser.set_defaults(func=handle_decode_response)
    
    # esp32command
    esp32command_parser = decode_subparsers.add_parser('esp32command', help='Proto "Esp32Command" message')
    esp32command_parser.set_defaults(func=handle_decode_esp32command)
     
    decode_parser.add_argument('data', type=str, help="Protobuf serialized data in hex format")
    
    return decode_parser

def handle_decode_measurement(args):
    data = bytes.fromhex(args.data)
    vals = decode_measurement(data)
    print(vals)

def handle_decode_response(args):
    data = bytes.fromhex(args.data)
    vals = decode_response(data)
    print(vals)

def handle_decode_esp32command(args):
    data = bytes.fromhex(args.data)
    vals = decode_esp32command(data)
    print(vals)