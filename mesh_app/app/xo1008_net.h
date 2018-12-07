
#ifndef _XO1008_NET_H_
#define _XO1008_NET_H_

#include "honyar_types.h"

#define XO1008_PROTOCOL_VERSION "1.0"

void xo1008_net_init(void);

void xo1008_net_upload(uint8_t *data, uint32_t len);

#endif
