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
 *
 * Author: Yu He
 * Link: https://github.com/chinaheyu/protocol
 */

#ifndef PROTOCOL_PROTOCOL_H
#define PROTOCOL_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define PROTOCOL_HEADER                         (0xA5)

typedef enum
{
    STEP_HEADER_SOF  = 0,
    STEP_CMD_LOW     = 1,
    STEP_CMD_HIGH    = 2,
    STEP_LENGTH_LOW  = 3,
    STEP_LENGTH_HIGH = 4,
    STEP_HEADER_CRC8 = 5,
    STEP_DATA_CRC16  = 6,
} protocol_unpack_step_e;

typedef struct
{
    /* Unpacked result. */
    uint16_t                    cmd_id;
    uint16_t                    data_len;
    uint8_t*                    data;

    /* Unpack state. */
    uint16_t                    max_data_length;
    uint8_t*                    protocol_packet_ptr;
    uint32_t                    protocol_packet_size;
    protocol_unpack_step_e      unpack_step;
    uint32_t                    index;
} protocol_stream_t;

/**
 * Test whether the device supports the protocol.
 * @return true or false
 */
extern bool protocol_is_supported(void);

/**
 * Calculate the packaged frame size.
 * @param data_length The size of the data to be packed.
 * @return Returns the calculated frame size.
 */
extern int protocol_calculate_frame_size(uint16_t data_length);

/**
 * Pack the data into memory.
 * @param cmd_id The cmd_id part of the frame.
 * @param data The data part of the frame.
 * @param data_length The data length of the frame.
 * @param buffer The output memory.
 * @return Returns the frame size.
 */
extern uint32_t protocol_pack_data_to_buffer(uint16_t cmd_id, const uint8_t *data, uint16_t data_length, uint8_t *buffer);

/**
 * Create the unpack stream object, allocate memory and initialize.
 * @param max_data_size The maximum data length contained in the frame.
 * @param auto_reallocate Should the stream object be resized automatically. If false, memory will be allocated according to the maximum data length.
 * @return Pointer to the unpack object.
 */
extern protocol_stream_t *protocol_create_unpack_stream(uint16_t max_data_length, bool auto_reallocate);

/**
 * Prepare memory for upcoming packets.
 * @param unpack_stream Pointer to the unpack object.
 * @param data_length The data length of the frame.
 * @return Returns false when an error occurs, true otherwise.
 */
extern bool protocol_reallocate_unpack_stream(protocol_stream_t* unpack_stream, uint16_t data_length);

/**
 * Initialize the unpacking state of unpack stream object.
 * @param unpack_stream Pointer to the unpack object.
 */
extern void protocol_initialize_unpack_stream(protocol_stream_t* unpack_stream);

/**
 * Free the unpack stream object.
 * @param unpack_stream Pointer to the unpack object.
 */
extern void protocol_free_unpack_stream(protocol_stream_t* unpack_stream);

/**
 * Parse the byte stream.
 * @param unpack_stream Pointer to the unpack object.
 * @param byte The byte read from the byte stream.
 * @return Returns true when a data frame is successfully parsed, false otherwise.
 */
extern bool protocol_unpack_byte(protocol_stream_t* unpack_stream, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif //PROTOCOL_PROTOCOL_H
