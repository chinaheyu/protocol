import pathlib
import struct
from crc import Calculator, Configuration
from random import randbytes, randrange
import argparse
import sys

CRC8_CONFIG = {'width': 8,
               'polynomial': 0x31,
               'init_value': 0xff,
               'final_xor_value': 0x00,
               'reverse_input': True,
               'reverse_output': True}

CRC16_CONFIG = {'width': 16,
                'polynomial': 0x8005,
                'init_value': 0xffff,
                'final_xor_value': 0x0000,
                'reverse_input': True,
                'reverse_output': True}

crc8 = Calculator(Configuration(**CRC8_CONFIG))
crc16 = Calculator(Configuration(**CRC16_CONFIG))


class ProgressBar:
    def __init__(self, toolbar_width, prefix=''):
        self.width = toolbar_width
        self.value = 0
        sys.stdout.write("%s[%s]" % (prefix, " " * self.width))
        sys.stdout.flush()
        sys.stdout.write("\b" * (self.width + 1))

    def update(self):
        self.value += 1
        if self.value >= self.width:
            sys.stdout.write("]\n")
        else:
            sys.stdout.write("=")
            sys.stdout.flush()


def generate_frame(cmd_id, data=b''):
    global crc8, crc16
    if isinstance(cmd_id, bytes):
        cmd_id = int.from_bytes(cmd_id, 'little')
    frame = struct.pack('<BHH', 0xa5, cmd_id, len(data))
    frame += struct.pack('<B', crc8.checksum(frame))
    if len(data) > 0:
        frame += data
        frame += struct.pack('<H', crc16.checksum(frame))
    return frame


def main(args):
    example_count = args.count

    # generate positive examples
    bar = ProgressBar(40, 'GP: ')
    with open(args.path / 'positive_example.txt', 'w') as f:
        for i in range(example_count):
            cmd_id = int.from_bytes(randbytes(2), 'little')
            data = randbytes(randrange(0, 121))
            f.write(f'{cmd_id:x}\n')
            f.write(f'{data.hex(" ")}\n')
            if i != example_count - 1:
                f.write(f'{generate_frame(cmd_id, data).hex(" ")}\n')
            else:
                f.write(f'{generate_frame(cmd_id, data).hex(" ")}')

            if (i + 1) % (example_count // bar.width) == 0:
                bar.update()

    # generate negative examples
    bar = ProgressBar(40, 'GN: ')
    with open(args.path / 'negative_example.txt', 'w') as f:
        for i in range(example_count):
            cmd_id = int.from_bytes(randbytes(2), 'little')
            data = randbytes(randrange(0, 121))
            frame = bytearray(generate_frame(cmd_id, data))
            old_index = randrange(0, len(frame))
            old_value = frame[old_index]
            while True:
                new_value = randrange(0, 256)
                if new_value != old_value:
                    frame[old_index] = new_value
                    break
            f.write(f'{cmd_id:x}\n')
            f.write(f'{data.hex(" ")}\n')
            if i != example_count - 1:
                f.write(f'{frame.hex(" ")}\n')
            else:
                f.write(f'{frame.hex(" ")}')

            if (i + 1) % (example_count // bar.width) == 0:
                bar.update()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--count', type=int, default=10000)
    parser.add_argument('-p', '--path', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parent)
    main(parser.parse_args())
