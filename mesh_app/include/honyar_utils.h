#ifndef _HONYAR_UTILS_H_
#define _HONYAR_UTILS_H_

#include "c_types.h"

int32_t parse_http_head(uint8_t *buf, uint32_t len, uint32_t *offset);

void hex_printf(uint8_t *head, uint8_t *buf, uint32_t len);

int hy_hex2byte(unsigned char *dest, int dest_len, unsigned char *src, int src_len);

int32_t hy_byte2hex(unsigned char *dest, unsigned int dest_len, unsigned char *src, unsigned int src_len);

void honyar_memmove(void *dest, void *src, unsigned int len);

uint16_t modbus_crc16 (uint8_t *data, uint8_t len);

uint8_t bcd_to_hex(uint8_t data);

//make sure the data less than 100;
uint8_t hex_to_bcd(uint8_t data);

#endif
