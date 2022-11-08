#include <stdio.h>
#include <string.h>
#include "protocol_lite.h"


int main(int argc, char* argv[]) {
    uint8_t data[PROTOCOL_DATA_MAX_SIZE];
    uint8_t tx_buf[PROTOCOL_FRAME_MAX_SIZE];
    unpack_data_t unpack_data_obj;
    protocol_initialize_unpack_object(&unpack_data_obj);
    size_t data_size;


    for(;;) {
        memset(data, 0, PROTOCOL_DATA_MAX_SIZE);
        data_size = 0;
        for (int i = 0; i < PROTOCOL_DATA_MAX_SIZE; ++i) {
            char ch;
            scanf("%c", &ch);
            if ((ch == '\n') || (ch == '\r')) {
                break;
            }
            data[i] = ch;
            data_size = i + 1;
        }

        printf("Data size: %d\nFrame size: %d\nSending......\n", data_size, protocol_calculate_frame_size(data_size));

        size_t send_size = protocol_pack_data_to_buffer(0x0123, data, data_size, tx_buf);

        printf("Packed frame: ");
        for (int i = 0; i < send_size; ++i) {
            printf("%02x ", tx_buf[i]);
        }
        printf("\n");

        for (int i = 0; i < send_size; ++i) {
            if (protocol_unpack_byte(&unpack_data_obj, tx_buf[i]))
            {
                printf("Unpack success.\n");
                printf("cmd_id: 0x%04x\ndata: ", unpack_data_obj.cmd_id);
                for (int j = 0; j < unpack_data_obj.data_len; ++j) {
                    printf("%c", (char)unpack_data_obj.data[j]);
                }
                printf("\n");
            }
        }
    }

    return 0;
}