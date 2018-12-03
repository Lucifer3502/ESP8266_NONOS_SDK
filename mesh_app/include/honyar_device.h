
#ifndef _HONYAR_DEVICE_H_
#define _HONYAR_DEVICE_H_

#include "honyar_types.h"

#define HONYAR_DEVICE_SN_MAX_SIZE 32

uint8_t *honyar_device_get_sn(void);

int32_t honyar_device_set_sn(uint8_t *sn);

#endif
