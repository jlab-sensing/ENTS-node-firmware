from protobuf_encoding.encode import encode_repeated_power_deltas
from python.LoRaWan import calculate_lorawan_airtime

"""This is a python script that tests the maximum number of entries the ENTS can put in a Protobuf
message. It is limited by the number of bytes that can be sent over LoRaWAN SF9, which is a 53 byter payload"""


def test_max_proto_entries():
    entries = [
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
        {"ts": 100, "voltage_delta": 100, "current_delta": 100},
    ]

    serialized = encode_repeated_power_deltas(logger_id=1, cell_id=2, entries=entries)

    print(f"Serialized data size: {len(serialized)} bytes")
    print(f"Airtime: {calculate_lorawan_airtime(pl_bytes=len(serialized))} ms")
    print(f"SF9 payload size: {53} bytes, and maximum airtime {185.3} ms")
    print(f"SF7 payload size: {222} bytes, and maximum airtime {368.9} ms")


if __name__ == "__main__":
    test_max_proto_entries()
