#ifndef _HONYAR_UTILS_H_
#define _HONYAR_UTILS_H_

#include "c_types.h"

int32_t parse_http_head(uint8_t *buf, uint32_t len, uint32_t *offset);

void hex_printf(uint8_t *head, uint8_t *buf, uint32_t len);

int hy_hex2byte(unsigned char *dest, int dest_len, unsigned char *src, int src_len);

int32_t hy_byte2hex(unsigned char *dest, int dest_len, unsigned char *src, int src_len);

void honyar_memmove(void *dest, void *src, unsigned int len);

#endif
