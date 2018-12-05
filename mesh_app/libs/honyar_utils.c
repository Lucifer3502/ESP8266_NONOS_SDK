

#include "honyar_common.h"

#define HTTP_TAIL_TAG  "\r\n\r\n"

int32_t ICACHE_FLASH_ATTR
parse_http_head(uint8_t *buf, uint32_t len, uint32_t *offset)
{
    uint32_t tail_size = strlen(HTTP_TAIL_TAG);
    uint32_t tag = 0;
    
    if(!buf || !len) {
        return -1;
    }

    while(len - tag >= tail_size) {
        if(memcmp(buf + tag, HTTP_TAIL_TAG, tail_size)) {
            tag++;
            continue;
        }
        *offset = tag + tail_size;
        return 0;
    }

    return -1;
}

void ICACHE_FLASH_ATTR
hex_printf(uint8_t *head, uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    
    if(head) {
        hy_printf("%s\r\n", head);
    }
    if(!buf) {
        return;
    }

    for(i = 0; i < len; i++) {
        hy_printf("%02X ", buf[i]);
    }
    hy_printf("\r\n");
}

int ICACHE_FLASH_ATTR
hy_hex2byte(unsigned char *dest, int dest_len, unsigned char *src, int src_len)
{
    if((src_len % 2) || (dest_len < src_len / 2))
        return -1;

    int i;
    int j;
    for(i = 0, j = 0; i < src_len; i++, j++) {
        if(src[i] >= '0' && src[i] <= '9') {
            dest[j] = src[i] - '0';
        } else if(src[i] >= 'a' && src[i] <= 'f') {
            dest[j] = src[i] - 'a' + 10;
        } else if(src[i] >= 'A' && src[i] <= 'F') {
            dest[j] = src[i] - 'A' + 10;
        } else {
            return -1;
        }
        i++;
        unsigned char tmp = dest[j] << 4;
        if(src[i] >= '0' && src[i] <= '9') {
            dest[j] = tmp+ (src[i] - '0');
        } else if(src[i] >= 'a' &&  src[i] <= 'f') {
            dest[j] = tmp + (src[i] - 'a' + 10);
        } else if(src[i] >= 'A' && src[i] <= 'F') {
            dest[j] = tmp + (src[i] - 'A' + 10);
        } else {
            return -1;
        }
    }
    return 0;
}

int32_t ICACHE_FLASH_ATTR
hy_byte2hex(unsigned char *dest, unsigned int dest_len, unsigned char *src, unsigned int src_len)
{
    uint32_t i;
    if(dest_len < src_len * 2 || !dest || !src) {
        return -1;
    }

    for(i = 0; i < src_len; i++) {
        os_sprintf((char *)&dest[i * 2], "%02X", src[i]);
    }

    return 0;
}

void ICACHE_FLASH_ATTR
honyar_memmove(void *dest, void *src, unsigned int len)
{
    uint32_t i = 0;
    uint8_t *d = dest;
    uint8_t *s = src;
    for(i = 0; i < len; i++) {
        d[i] = s[i];
    }
}

uint16_t ICACHE_FLASH_ATTR
modbus_crc16 (uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint32_t i = 0;
    uint32_t j = 0;

    for ( i = 0; i < len; i++) {
        crc = crc ^ data[i];
        for (j = 0; j < 8; j++) {
           if(crc & 0x0001) {
               crc = crc >> 1;
               crc = crc ^ 0xa001;
          } else {
                crc = crc >> 1;
          }
        }
    }

    return crc;
}

uint8_t ICACHE_FLASH_ATTR
bcd_to_hex(uint8_t data)
{
    return ((data >> 4) * 10  + (data & 0x0f));
}

uint8_t ICACHE_FLASH_ATTR
hex_to_bcd(uint8_t data)
{
    return (((data / 10) << 4) + (data % 10));
}



