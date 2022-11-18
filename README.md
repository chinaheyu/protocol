# protocol

## Frame format

```
+-----------------------+---------------+----------------------------+
| frame_header (6-byte) | data (n-byte) | frame_tail (2-byte, CRC16) |
+-----------------------+---------------+----------------------------+
```

Format of frame_header:

```
+--------------+-----------------+----------------------+---------------+
| sof (1-byte) | cmd_id (2-byte) | data_length (2-byte) | crc8 (1-byte) |
+--------------+-----------------+----------------------+---------------+
```

The frame_tail is the checksum of the total frame. If `data_length == 0`, frame_tail will be dropped.

## Usage

Pack data to memory.

```c
uint32_t protocol_pack_data_to_buffer(uint16_t cmd_id, const uint8_t *data, uint16_t len, uint8_t *buffer);
```

Unpack from byte stream.

```c
uint32_t protocol_unpack_byte(unpack_data_t* unpack_obj, uint8_t byte);
```

## Python wrapper

```python
import protocol_lite as pl


data_frames = pl.pack_data(0x0123) + pl.pack_data(0x4567, b'Hello World!')

stream = pl.UnpackStream()
for frame in stream.unpack_data(data_frames):
    print(frame)
```

output

```
cmd_id: 0x0123, empty data.
cmd_id: 0x4567, data: 48 65 6c 6c 6f 20 57 6f 72 6c 64 21.
```
