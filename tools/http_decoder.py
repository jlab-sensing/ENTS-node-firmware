#!/usr/bin/env python

"""
HTTP decoding server.
"""

import argparse
from http.server import BaseHTTPRequestHandler, HTTPServer

from ents.proto.sensor import parse_sensor_measurement


class DirtvizRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b"Hello World.\n")

    def do_POST(self):
        # print request
        content_length = int(self.headers['Content-Length'])
        data = self.rfile.read(content_length)

        print("Headers:")
        print(self.headers)
        print()

        print(f"Received data {type(data)}:")
        print(data)
        print()

        meas = parse_sensor_measurement(data)
        print("Decoded data:")
        print(meas)
        print()

        # send response
        self.send_response(200)
        self.send_header('Content-type', 'text/octet-stream')
        self.end_headers()
        self.wfile.write(b"The world is your oyster!")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Simple HTTP server with POST request handling.')
    parser.add_argument('--address', type=str, default='',
                        help='Server address (default: all available interfaces)')
    parser.add_argument('--port', type=int, default=8080,
                        help='Server port (default: 8080)')
    args = parser.parse_args()

    server_address = (args.address, args.port)
    httpd = HTTPServer(server_address, DirtvizRequestHandler)
    httpd.serve_forever()
