#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define BIT(x)  (1u << (x))

struct CRC_INFO {
    const char* name;
    unsigned char width;
    unsigned int poly;
    unsigned int init;
    bool refin;
    bool refout;
    unsigned int xorout;
};

unsigned int table[256];

void printfTable(FILE* fp, int n) {
    int i;
    char fmt[20];
    for (i = 0; i < 256; ++i) {
        if (i % 16 == 0)
            fprintf(fp, "\n    ");
        sprintf(fmt, "0x%%0%dx, ", n);
        fprintf(fp, fmt, table[i]);
    }
}

//位逆转
unsigned int reflected(unsigned int input, unsigned char bits) {
    unsigned int var = 0;
    while (bits--) {
        var <<= 1;
        if (input & 0x01)
            var |= 1;
        input >>= 1;
    }
    return var;
}


void crc_table_init(const struct CRC_INFO *info) {
    unsigned short i;
    unsigned char j;
    unsigned int poly, value;
    unsigned int valid_bits = (2 << (info->width - 1)) - 1;

    if (info->refin) {
        //逆序LSB输入
        //poly 以及init 都要先逆序, table 的 init = 0;
        poly = reflected(info->poly, info->width);

        for (i = 0; i < 256; i++) {
            value = i;
            for (j = 0; j < 8; j++) {
                if (value & 1)
                    value = (value >> 1) ^ poly;
                else
                    value = (value >> 1);
            }
            table[i] = value & valid_bits;
        }
    } else {
        //正序MSB输入
        //如果位数小于8，poly要左移到最高位
        poly = info->width < 8 ? info->poly << (8 - info->width) : info->poly;
        unsigned int bit = info->width > 8 ? BIT(info->width - 1) : 0x80;

        for (i = 0; i < 256; i++) {
            value = info->width > 8 ? i << (info->width - 8) : i;

            for (j = 0; j < 8; j++) {
                if (value & bit)
                    value = (value << 1) ^ poly;
                else
                    value = (value << 1);
            }

            //如果width < 8，那么实际上，crc是在高width位的，需要右移 8 - width
            //但是为了方便后续异或（还是要移位到最高位与*ptr的bit7对齐），所以不处理
            // if (info->width < 8)
            //	  value >>=  8 - info->width;
            // table[i] = value & (2 << (info->width - 1)) - 1);
            table[i] = value & (info->width < 8 ? 0xff : valid_bits);
        }
    }
}


unsigned int crc(const struct CRC_INFO *info, const unsigned char *ptr, unsigned int len) {
    unsigned int value;
    unsigned char high;

    if (info->refin) {
        //逆序 LSB 输入
        value = reflected(info->init, info->width);

        // 为了减少移位等操作，width大于8和小于8的分开处理
        //当width <= 8时，
        // value = (value >> 8) ^ table[value & 0xff ^ *ptr++];
        //可简化为
        // value = table[value ^ *ptr++];
        if (info->width > 8) {
            while (len--) {
                value = (value >> 8) ^ table[value & 0xff ^ *ptr++];
            }
        } else
            while (len--) {
                value = table[value ^ *ptr++];
            }
    } else {
        //正序 MSB 输入
        //为了减少移位等操作，width大于8和小于8的分开处理
        //当width <= 8时，
        // high = value >> (info->width - 8);
        // value = (value << 8) ^ table[high ^ *ptr++];
        //可简化为
        // value = table[value ^ *ptr++];

        if (info->width > 8) {
            value = info->init;

            while (len--) {
                high = value >> (info->width - 8);
                value = (value << 8) ^ table[high ^ *ptr++];
            }
        } else {
            value = info->init << (8 - info->width);
            while (len--) {
                value = table[value ^ *ptr++];
            }

            //位数小于8时，data在高width位，要右移
            value >>= 8 - info->width;
        }
    }

    //逆序输出
    if (info->refout != info->refin)
        value = reflected(value, info->width);

    //异或输出
    value ^= info->xorout;

    return value & ((2 << (info->width - 1)) - 1);
}

//struct CRC_INFO {
//    const char* name;
//    unsigned char width;
//    unsigned int poly;
//    unsigned int init;
//    bool refin;
//    bool refout;
//    unsigned int xorout;
//};
const struct CRC_INFO crc_struct[] = {
        {"CRC8",  8,  0x31,       0xff,       true, true, 0x00},           //CRC8
        {"CRC16", 16, 0x8005,     0xffff,     true, true, 0x0000},         //CRC16
        {"CRC32", 32, 0x04C11DB7, 0xffffffff, true, true, 0x00000000},     //CRC32
};

int main() {

    unsigned int i;
    const char *str = "HelloWorld!";
    unsigned int value;

    FILE* fp = fopen("crc_table.txt", "w");

    fprintf(fp, "input string is \"%s\"\n", str);

    for (i = 0; i < sizeof(crc_struct) / sizeof(crc_struct[0]); i++) {
        crc_table_init(&crc_struct[i]);

        fprintf(fp, "%s Table\n{", crc_struct[i].name);
        printfTable(fp, crc_struct[i].width / 4);
        fprintf(fp, "\n};\n\n");

        value = crc(&crc_struct[i], (unsigned char *) str, strlen(str));

        fprintf(fp, "%s(\"%s\") = 0x%x\n", crc_struct[i].name, str, value);
        fprintf(fp, "=======================================================\n");
    }

    fclose(fp);

    system("crc_table.txt");
}