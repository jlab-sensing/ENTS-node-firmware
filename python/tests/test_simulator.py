"""Testing simulation code

Tests the functionality of the NodeSimulator class uploading data to a sample
server. A HTTP server to handle post request is started on a per test basis at
localhost:8080. Ensure there is not another process using this port.
"""

import unittest
import sys
import os

from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from multiprocessing import Process
from datetime import datetime, timedelta
from time import sleep

# Ensure src is in path for subprocesses
sys.path.append(os.path.abspath("src"))

from ents.simulator.node import NodeSimulator


class Backend(BaseHTTPRequestHandler):
    """Backend simulator"""

    def log_message(self, format, *args):
        """Override to not print to stdout"""
        pass

    def do_POST(self):
        """Return 200 from a POST request"""

        # print request
        # content_length = int(self.headers["Content-Length"])
        # data = self.rfile.read(content_length)

        # send response
        self.send_response(200)
        self.send_header("Content-type", "text/octet-stream")
        self.end_headers()
        self.wfile.write(b"The world is your oyster!")


def run_server():
    """Run the simulated HTTP server on localhost:8080"""

    # run server
    server = ThreadingHTTPServer(("localhost", 8080), Backend)
    server.serve_forever()


class TestNodeSimulator(unittest.TestCase):
    url = "http://localhost:8080/"

    def setUp(self):
        """Start webserver in separate process"""

        self.backend = Process(target=run_server)
        self.backend.start()
        # give server time to start
        sleep(1)

    def tearDown(self):
        """Terminate process"""

        self.backend.terminate()

    def test_power(self):
        sim = NodeSimulator(1, 2, "power")
        ts = int(datetime(2025, 5, 7).timestamp())
        sim.measure(ts)
        sim.send_next(self.url)

    def test_teros12(self):
        sim = NodeSimulator(1, 2, "teros12")
        ts = int(datetime(2025, 5, 7).timestamp())
        sim.measure(ts)
        sim.send_next(self.url)

    def test_teros21(self):
        sim = NodeSimulator(1, 2, "teros21")
        ts = int(datetime(2025, 5, 7).timestamp())
        sim.measure(ts)
        sim.send_next(self.url)

    def test_bme280(self):
        sim = NodeSimulator(1, 2, "bme280")
        ts = int(datetime(2025, 5, 7).timestamp())
        sim.measure(ts)
        sim.send_next(self.url)

    def test_max_connections(self):
        """Covers the case of streaming or batch mode with more than ~1024
        uploads. Can cause an error with the max number of file descriptors
        """

        sim = NodeSimulator(1, 2, "bme280")
        ts_start = datetime(2025, 5, 7)

        for i in range(2000):
            ts = int((ts_start + timedelta(days=i)).timestamp())
            sim.measure(ts)
            sim.send_next(self.url)


if __name__ == "__main__":
    # run unittests
    unittest.main()
