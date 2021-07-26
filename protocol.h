/*
 * Frame format:
 * +-----------------------+-----------------+---------------+----------------------------+
 * | frame_header (5-byte) | cmd_id (2-byte) | data (n-byte) | frame_tail (2-byte，CRC16) |
 * +-----------------------+-----------------+---------------+----------------------------+
 *
 * Format of frame_header:
 * +--------------+----------------------+--------------+---------------+
 * | sof (1-byte) | data_length (2-byte) | seq (1-byte) | crc8 (1-byte) |
 * +--------------+----------------------+--------------+---------------+
 *
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

//******************************************************************************************
//!                           CONFIGURE MACRO
//******************************************************************************************
#define PROTOCOL_FRAME_MAX_SIZE                 128
#define PROTOCOL_HEADER                         0xA5
#define PROTOCOL_HEADER_SIZE                    sizeof(frame_header_t)
#define PROTOCOL_CMD_SIZE                       2
#define PROTOCOL_CRC16_SIZE                     2
#define PROTOCOL_HEADER_CRC_LEN                 (PROTOCOL_HEADER_SIZE + PROTOCOL_CRC16_SIZE)
#define PROTOCOL_HEADER_CRC_CMDID_LEN           (PROTOCOL_HEADER_SIZE + PROTOCOL_CRC16_SIZE + sizeof(uint16_t))
#define PROTOCOL_HEADER_CMDID_LEN               (PROTOCOL_HEADER_SIZE + sizeof(uint16_t))

//******************************************************************************************
//!                           PUBLIC TYPE
//******************************************************************************************
typedef int  (*protocol_read_function_t)(uint8_t* p_data, int len);
typedef void (*protocol_write_function_t)(uint8_t* p_data, int len);
typedef void (*protocol_cmd_callback_t)(uint16_t cmd_id, uint8_t *p_data, uint16_t len);

typedef struct {
    uint8_t  sof;
    uint16_t data_length;
    uint8_t  seq;
    uint8_t  crc8;
}__attribute__((packed)) frame_header_t;

typedef enum
{
    STEP_HEADER_SOF  = 0,
    STEP_LENGTH_LOW  = 1,
    STEP_LENGTH_HIGH = 2,
    STEP_FRAME_SEQ   = 3,
    STEP_HEADER_CRC8 = 4,
    STEP_DATA_CRC16  = 5,
} unpack_step_e;

typedef struct
{
    frame_header_t *p_header;
    uint16_t       data_len;
    uint8_t        protocol_packet[PROTOCOL_FRAME_MAX_SIZE];
    unpack_step_e  unpack_step;
    uint16_t       index;
} unpack_data_t;

typedef struct
{
    protocol_read_function_t  read_function;
    protocol_write_function_t write_function;
    protocol_cmd_callback_t   cmd_callback;
    unpack_data_t             unpack_obj;
    uint8_t                   send_seq;
} protocol_t;

//******************************************************************************************
//!                           PUBLIC API
//******************************************************************************************
/**
 * Init a protocol struct.
 * @param p_protocol     pointer of protocol struct.
 * @param read_function  function that read the data and return the successful count.
 * @param write_function function that write the data.
 * @param cmd_callback   callback function when received a data frame.
 */
extern void protocol_init(protocol_t* p_protocol, protocol_read_function_t read_function, protocol_write_function_t write_function, protocol_cmd_callback_t cmd_callback);

/**
 * Transmit data through the protocol.
 * @param p_protocol pointer of protocol struct.
 * @param cmd_id     cmd_id.
 * @param p_buf      pointer of the data.
 * @param len        length of the data.
 */
extern void protocol_transmit(protocol_t* p_protocol, uint16_t cmd_id, const uint8_t *p_buf, uint16_t len);

/**
 * Unpack data through the read function, you need to call this function in a infinite loop.
 * @param p_protocol pointer of protocol struct.
 */
extern void protocol_unpack_data(protocol_t* p_protocol);

#endif //PROTOCOL_H
