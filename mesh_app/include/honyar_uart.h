
#ifndef _HONYAR_UART_H_
#define _HONYAR_UART_H_

#include "c_types.h"


int32_t honyar_uart_init(uint32_t baudrate);

uint32_t honyar_uart_write(const uint8_t *buf, uint32_t len);

uint32_t honyar_uart_read(uint8_t *buf, uint32_t len, uint32_t tm_out);

#endif

