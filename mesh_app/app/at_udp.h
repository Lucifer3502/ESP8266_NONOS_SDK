#ifndef _AT_UDP_H_
#define _AT_UDP_H_

#include "honyar_types.h"

int32_t at_udp_send(uint8_t *data, uint16_t len);

void at_udp_init(void);

#endif
