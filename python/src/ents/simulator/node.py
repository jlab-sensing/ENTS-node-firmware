from math import sin

import requests
import numpy as np

from ..proto.encode import (
    encode_power_measurement,
    encode_teros12_measurement,
    encode_teros21_measurement,
    encode_bme280_measurement,
)

from ..proto.sensor import encode_repeated_sensor_measurements


class NodeSimulator:
    """Simulation class to simulate measurements for different sensors"""

    # temporary storage for measurements to be uploaded
    measurement_buffer: list[bytes] = []
    # all measurements uploaded
    measurements: list[bytes] = []
    # all responses
    responses: list[str] = []

    # metrics for uploads
    metrics: dict[str, int] = {
        "total_requests": 0,
        "failed_requests": 0,
        "successful_requests": 0,
    }

    latency: list[float] = []

    def __init__(self, cell: int, logger: int, sensors: list[str], fn=sin):
        self.cell = cell
        self.logger = logger
        self.sensors = sensors
        self.fn = fn
        self.session = requests.Session()

    def __str__(self):
        """String representation of the simulation class

        Shows the current upload metrics
        """
        avg = np.array(self.latency).mean()

        last = 0
        if len(self.latency) > 0:
            last = self.latency[-1]

        return "total: {}, failed: {}, avg (ms): {}, last (ms): {}".format(
            self.metrics["total_requests"],
            self.metrics["failed_requests"],
            avg * 100,
            last * 100,
        )

    def send_next(self, url: str) -> bool:
        """Sends measurements to a dirtviz instance

        Args:
            url: URL of the dirtviz instance

        Returns:
            True if there are measurements to send, False otherwise
        """

        # get next measurement
        try:
            meas = self.measurements.pop()
        except IndexError as _:
            return False

        headers = {"Content-Type": "application/octet-stream"}
        result = self.session.post(url, data=meas, headers=headers)

        # store result
        self.responses.append(result.text)
        self.metrics["total_requests"] += 1
        if result.status_code == 200:
            self.metrics["successful_requests"] += 1
        else:
            self.metrics["failed_requests"] += 1
        self.latency.append(result.elapsed.total_seconds())

        return True

    def measure(self, ts: int):
        """Simulate measurements

        Args:
            ts: Timestamp of the measurement

        Returns:
            None
        """

        if "power" in self.sensors:

            voltage = self.fn(ts) * 2
            current = self.fn(ts) * 0.5

            meas = encode_power_measurement(
                ts=ts,
                cell_id=self.cell,
                logger_id=self.logger,
                voltage=voltage,
                current=current,
            )
            self.measurements.append(meas)
            self.measurement_buffer.append(meas)

        if "teros12" in self.sensors:

            vwc_raw = self.fn(ts) * 300 + 2500
            vwc_adj = self.fn(ts) * 0.05 + 0.2
            temp = self.fn(ts) * 5 + 25
            ec = self.fn(ts) * 4 + 15

            meas = encode_teros12_measurement(
                ts=ts,
                cell_id=self.cell,
                logger_id=self.logger,
                vwc_raw=vwc_raw,
                vwc_adj=vwc_adj,
                temp=temp,
                ec=int(ec),
            )
            self.measurements.append(meas)
            self.measurement_buffer.append(meas)

        if "teros21" in self.sensors:

            matric_pot = self.fn(ts) * 200 + 1000
            temp = self.fn(ts) * 5 + 25

            meas = encode_teros21_measurement(
                ts=ts,
                cell_id=self.cell,
                logger_id=self.logger,
                matric_pot=matric_pot,
                temp=temp,
            )
            self.measurements.append(meas)
            self.measurement_buffer.append(meas)

        if "bme280" in self.sensors:

            temp = self.fn(ts) * 50 + 250
            humidity = self.fn(ts) * 200 + 2000
            pressure = self.fn(ts) * 2000 + 43000

            meas = encode_bme280_measurement(
                ts=ts,
                cell_id=self.cell,
                logger_id=self.logger,
                temperature=int(temp),
                humidity=int(humidity),
                pressure=int(pressure),
            )
            self.measurements.append(meas)
            self.measurement_buffer.append(meas)


class NodeSimulatorGeneric:
    """Simulation class to simulate measurements for different sensors"""

    # temporary storage for measurements to be uploaded
    measurement_buffer: list[bytes] = []
    # all measurements uploaded
    measurements: list[bytes] = []
    # all responses
    responses: list[str] = []
    # all requests in format (headers, body)
    requests: list[tuple[str, str]] = []

    # metrics for uploads
    metrics: dict[str, int] = {
        "total_requests": 0,
        "failed_requests": 0,
        "successful_requests": 0,
    }

    latency: list[float] = []

    def __init__(
        self, cell: int, logger: int, sensors: list[str], _min=-1, _max=1, fn=sin
    ):
        """Initializes the simulation class.

        Args:
            cell: Cell ID of the node.
            logger: Logger ID of the node.
            sensors: List of sensors to simulate.
            _min: Minimum value for the simulated sensor data.
            _max: Maximum value for the simulated sensor data.
            fn: Function to generate the simulated sensor data.
        """

        self.cell = cell
        self.logger = logger
        self.sensors = sensors
        self.fn = fn
        self._min = _min
        self._max = _max

    def __str__(self):
        """String representation of the simulation class

        Shows the current upload metrics
        """
        avg = np.array(self.latency).mean()

        last = 0
        if len(self.latency) > 0:
            last = self.latency[-1]

        return "total: {}, failed: {}, avg (ms): {}, last (ms): {}".format(
            self.metrics["total_requests"],
            self.metrics["failed_requests"],
            avg * 100,
            last * 100,
        )

    def send_next(self, url: str) -> bool:
        """Sends measurements to a dirtviz instance

        Args:
            url: URL of the dirtviz instance

        Returns:
            True if there are measurements to send, False otherwise
        """

        # get next measurement
        try:
            meas = self.measurement_buffer.pop()
        except IndexError as _:
            return False

        headers = {
            "Content-Type": "application/octet-stream",
            "SensorVersion": "2",
        }
        result = requests.post(url, data=meas, headers=headers)

        # store result
        self.requests.append((result.request.headers, result.request.body))
        self.responses.append(result.text)
        self.metrics["total_requests"] += 1
        if result.status_code == 200:
            self.metrics["successful_requests"] += 1
        else:
            self.metrics["failed_requests"] += 1
        self.latency.append(result.elapsed.total_seconds())

        return True

    def measure(self, ts: int):
        """Simulate measurements

        Args:
            ts: Timestamp of the measurement
        """

        meas = {
            "meta": {
                "ts": ts,
                "loggerId": self.logger,
                "cellId": self.cell,
            },
            "measurements": [],
        }

        scale = (self._max - self._min) / 2
        offset = (self._max + self._min) / 2

        for s in self.sensors:
            meas["measurements"].append(
                {
                    "type": s,
                    "decimal": self.fn(ts) * scale + offset,
                }
            )

        serialized = encode_repeated_sensor_measurements(meas)
        self.measurements.append(serialized)
        self.measurement_buffer.append(serialized)

    def last_measurement(self) -> bytes:
        """Gets the last encoded measurement.


        Returns:
            Last encoded measurement.
        """

        return self.measurements[-1]

    def last_request(self) -> str:
        """Gets the last sent request.

        Returns:
            Formatted headers and body.
        """

        headers = self.requests[-1][0]
        body = self.requests[-1][1]
        request_str = f"{headers}\n\n{body}"
        return request_str

    def last_response(self) -> str:
        """Gets the last response.

        Returns:
            Response from server.
        """

        return self.responses[-1]
