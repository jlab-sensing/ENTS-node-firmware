#!/usr/bin/env python

"""
@file http_server.py

HTTP server used for testing functionality of the ESP32 WiFi interface.

@author John Madden <jmadden173@pm.me>
@date 2024-01-01
"""

import argparse
from http.server import BaseHTTPRequestHandler, HTTPServer


class DirtvizRequestHandler(BaseHTTPRequestHandler):
    simulate_error = 0

    def do_POST(self):
        # print request
        content_length = int(self.headers['Content-Length'])
        data = self.rfile.read(content_length)

        print("Headers:")
        print(self.headers)
        print()
        print(f"Received data {type(data)}:")
        print(data)

        # send response
        if self.server.simulate_error:
            self.send_response(418)
            self.send_header('Content-type', 'text/octet-stream')
            self.end_headers()
            self.wfile.write(b"I'm a teapot not a coffee maker!")
        else:
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
    parser.add_argument('--error', action='store_true',
                        help='Send error on response')
    args = parser.parse_args()

    server_address = (args.address, args.port)
    httpd = HTTPServer(server_address, DirtvizRequestHandler)
    httpd.simulate_error = args.error
    httpd.serve_forever()
