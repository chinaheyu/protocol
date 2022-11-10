/*
 * Frame format:
 * +-----------------------+---------------+----------------------------+
 * | frame_header (6-byte) | data (n-byte) | frame_tail (2-byte, CRC16) |
 * +-----------------------+---------------+----------------------------+
 *
 * Format of frame_header:
 * +--------------+-----------------+----------------------+---------------+
 * | sof (1-byte) | cmd_id (2-byte) | data_length (2-byte) | crc8 (1-byte) |
 * +--------------+-----------------+----------------------+---------------+
 *
 * The frame_tail is the checksum of the total frame.
 * If data_length == 0, frame_tail will be dropped.
 */

#ifndef PROTOCOL_PROTOCOL_LITE_H
#define PROTOCOL_PROTOCOL_LITE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define PROTOCOL_HEADER                         0xA5
#define PROTOCOL_FRAME_MAX_SIZE                 128
#define PROTOCOL_HEADER_SIZE                    6
#define PROTOCOL_HEADER_CRC_SIZE                (PROTOCOL_HEADER_SIZE + 2)
#define PROTOCOL_DATA_MAX_SIZE                  (PROTOCOL_FRAME_MAX_SIZE - PROTOCOL_HEADER_CRC_SIZE)


#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint8_t  sof;
    union {
        uint16_t cmd_id;
        uint8_t _cmd_id[2];
    };
    union {
        uint16_t data_length;
        uint8_t _data_length[2];
    };
    uint8_t  crc8;
} frame_header_t;

#pragma pack(pop)

typedef enum
{
    STEP_HEADER_SOF  = 0,
    STEP_CMD_LOW     = 1,
    STEP_CMD_HIGH    = 2,
    STEP_LENGTH_LOW  = 3,
    STEP_LENGTH_HIGH = 4,
    STEP_HEADER_CRC8 = 5,
    STEP_DATA_CRC16  = 6,
} unpack_step_e;

typedef struct
{
    uint16_t        cmd_id;
    uint16_t        data_len;
    uint8_t*        data;

    uint8_t         protocol_packet[PROTOCOL_FRAME_MAX_SIZE];
    unpack_step_e   unpack_step;
    uint16_t        index;
} unpack_data_t;

// API

extern uint32_t protocol_calculate_frame_size(uint32_t data_size);

extern uint32_t protocol_pack_data_to_buffer(uint16_t cmd_id, const uint8_t *data, uint16_t len, uint8_t *buffer);

extern void protocol_initialize_unpack_object(unpack_data_t* unpack_obj);

extern uint32_t protocol_unpack_byte(unpack_data_t* unpack_obj, uint8_t byte);

// Utils

extern char get_endianness();

extern uint16_t swap_uint16( uint16_t val );

extern int16_t swap_int16( int16_t val );

extern uint32_t swap_uint32( uint32_t val );

extern int32_t swap_int32( int32_t val );

extern uint64_t swap_uint64( uint64_t val );

extern int64_t swap_int64( int64_t val );

#ifdef __cplusplus
}
#endif

#endif //PROTOCOL_PROTOCOL_LITE_H
