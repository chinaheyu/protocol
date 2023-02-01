#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/protocol.h"

#define PROTOCOL_FRAME_MAX_SIZE 128
#define PROTOCOL_DATA_MAX_SIZE 120
int test_result;

#define TEST_SUCCESS do { \
    test_result = 0; \
    printf("SUCCESS\n"); \
} while(0)

#define TEST_FAIL do { \
    test_result = -1; \
    printf("FAIL\n"); \
} while(0)

uint8_t hex_to_byte(char hi, char lo) {
    char hex_string[3];
    hex_string[0] = hi;
    hex_string[1] = lo;
    hex_string[2] = '\0';

    return strtoul(hex_string, NULL, 16);
}

uint16_t file_read_hex_line(FILE* fp, uint8_t* out, uint16_t max_size)
{
    uint16_t data_len = 0;
    for (int i = 0; i < max_size; ++i) {
        char ch1 = fgetc(fp);
        if ((ch1 == '\r') || (ch1 == '\n') || (ch1 ==  EOF))
        {
            break;
        }

        char ch2 = fgetc(fp);
        if ((ch2 == '\r') || (ch2 == '\n') || (ch2 ==  EOF))
        {
            break;
        }

        uint8_t byte = hex_to_byte(ch1, ch2);
        out[i] = byte;
        data_len = i + 1;

        char ch3 = fgetc(fp);
        if (ch3 != ' ')
        {
            break;
        }
    }
    return data_len;
}

int pack_test(FILE* fp)
{
    char line_buffer[2000];

    uint16_t cmd_id;
    uint16_t data_len;
    uint8_t data[PROTOCOL_DATA_MAX_SIZE];
    int frame_size;
    uint8_t frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    uint8_t test_frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    fseek(fp, 0, SEEK_SET);

    char* line;
    int cnt = 0;
    while (1)
    {
        line = fgets(line_buffer, sizeof line_buffer, fp);
        if(line == NULL)
            break;
        cmd_id = strtoul(line, NULL, 16);

        data_len = file_read_hex_line(fp, data, PROTOCOL_DATA_MAX_SIZE);

        frame_size = file_read_hex_line(fp, frame_buffer, PROTOCOL_FRAME_MAX_SIZE);

        int test_frame_size = protocol_pack_data_to_buffer(cmd_id, data, data_len, test_frame_buffer);
        if (test_frame_size == PROTOCOL_ERROR)
            return 0;

        if (frame_size != test_frame_size)
            return 0;

        if(memcmp(frame_buffer, test_frame_buffer, frame_size) != 0)
            return 0;

        cnt++;
    }

    if (cnt == 0) {
        return 0;
    }

    printf("Test passed %d positive cases.\n", cnt);
    return 1;
}

int unpack_test(FILE* fp)
{
    char line_buffer[2000];

    uint16_t cmd_id;
    uint16_t data_len;
    uint8_t data[PROTOCOL_DATA_MAX_SIZE];
    int frame_size;
    uint8_t frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    protocol_stream_t* unpack_obj = protocol_create_unpack_stream(PROTOCOL_DATA_MAX_SIZE);

    fseek(fp, 0, SEEK_SET);

    char* line;
    int cnt = 0;
    int result;
    while (1)
    {
        line = fgets(line_buffer, sizeof line_buffer, fp);
        if(line == NULL)
            break;
        cmd_id = strtoul(line, NULL, 16);

        data_len = file_read_hex_line(fp, data, PROTOCOL_DATA_MAX_SIZE);

        frame_size = file_read_hex_line(fp, frame_buffer, PROTOCOL_FRAME_MAX_SIZE);

        result = 0;
        for (int i = 0; i < frame_size; ++i) {
            if (protocol_unpack_byte(unpack_obj, frame_buffer[i])) {
                if (unpack_obj->cmd_id != cmd_id)
                    return 0;

                if (unpack_obj->data_len != data_len)
                    return 0;

                if (memcmp(unpack_obj->data, data, data_len) != 0)
                    return 0;

                result = 1;
            }
        }

        if (result == 0)
            return 0;

        cnt++;
    }

    protocol_free_unpack_stream(unpack_obj);

    if (cnt == 0) {
        return 0;
    }

    printf("Test passed %d positive cases.\n", cnt);
    return 1;
}

int unpack_negative_test(FILE* fp)
{
    char line_buffer[2000];

    uint16_t cmd_id;
    uint16_t data_len;
    uint8_t data[PROTOCOL_DATA_MAX_SIZE];
    int frame_size;
    uint8_t frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    protocol_stream_t* unpack_obj = protocol_create_unpack_stream(PROTOCOL_DATA_MAX_SIZE);

    fseek(fp, 0, SEEK_SET);

    char* line;
    int cnt = 0;
    while (1)
    {
        line = fgets(line_buffer, sizeof line_buffer, fp);
        if(line == NULL)
            break;

        cmd_id = strtoul(line, NULL, 16);
        data_len = file_read_hex_line(fp, data, PROTOCOL_DATA_MAX_SIZE);
        frame_size = file_read_hex_line(fp, frame_buffer, PROTOCOL_FRAME_MAX_SIZE);

        for (int i = 0; i < frame_size; ++i) {
            if (protocol_unpack_byte(unpack_obj, frame_buffer[i])) {
                return 0;
            }
        }

        cnt++;
    }

    protocol_free_unpack_stream(unpack_obj);

    if (cnt == 0) {
        return 0;
    }

    printf("Test passed %d negative cases.\n", cnt);
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        TEST_FAIL;
        return test_result;
    }

    FILE* fp = fopen(argv[2], "r");

    int opt = atoi(argv[1]);
    switch (opt) {
        case 0:
        {
            // support test
            if (protocol_is_supported()) {
                TEST_SUCCESS;
            } else {
                TEST_FAIL;
            }
        }
        break;

        case 1:
        {
            // pack test
            if (pack_test(fp)) {
                TEST_SUCCESS;
            } else {
                TEST_FAIL;
            }
        }
        break;

        case 2:
        {
            // unpack test
            if (unpack_test(fp)) {
                TEST_SUCCESS;
            } else {
                TEST_FAIL;
            }
        }
        break;

        case 3:
        {
            // unpack test
            if (unpack_negative_test(fp)) {
                TEST_SUCCESS;
            } else {
                TEST_FAIL;
            }
        }
            break;

        default:
        {
            TEST_FAIL;
        }
    }

    fclose(fp);
    return test_result;
}