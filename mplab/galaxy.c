#include "app.h"
#include "galaxy.h"


unsigned short compute_crc( unsigned int *ptr_msg_body, int len_body) {
    int i;
    unsigned short crc = 0xffff; /* Initial Value */
    for (i = 0; i < len_body; i ++)
        crc = ((crc >> 8) & 0xFF) ^ table_crc[(crc ^ (*ptr_msg_body++ & 0xFF)) & 0xFF];
    return crc;
}
