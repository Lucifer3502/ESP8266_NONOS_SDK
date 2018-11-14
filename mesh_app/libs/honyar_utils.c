

#include "honyar_common.h"

#define HTTP_TAIL_TAG  "\r\n\r\n"

int32_t parse_http_head(uint8_t *buf, uint32_t len, uint32_t *offset)
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

void hex_printf(uint8_t *head, uint8_t *buf, uint32_t len)
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

int hex2byte(unsigned char *dest, int dest_len, unsigned char *src, int src_len)
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

int32_t byte2hex(unsigned char *dest, int dest_len, unsigned char *src, int src_len)
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

void honyar_memmove(void *dest, void *src, unsigned int len)
{
    uint32_t i = 0;
    uint8_t *d = dest;
    uint8_t *s = src;
    for(i = 0; i < len; i++) {
        d[i] = s[i];
    }
}


