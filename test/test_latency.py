import py_protocol as p
import serial
import time
import struct
import matplotlib.pyplot as plt
from tqdm import tqdm


def wait_response(serial_port):
    unpack_stream = p.UnpackStream()
    while True:
        for frame in unpack_stream(serial_port.read_all()):
            if frame.cmd_id == 0x0001:
                latency_us = int(time.time() * 1e6) - struct.unpack('<q', frame.data)[0]
                return latency_us


def main():
    serial_port = serial.Serial(input('Serial port name: '), int(input('Baudrate: ')))
    latency_result = []
    for i in tqdm(range(10000)):
        data = struct.pack('<q', int(time.time() * 1e6))
        serial_port.write(p.pack_data(0x0001, data))
        latency_result.append(wait_response(serial_port))
    plt.figure()
    plt.hist(latency_result, bins=20)
    plt.xlabel('Latency (us)')
    plt.ylabel('Probability')
    plt.show()


if __name__ == '__main__':
    main()
