
#ifndef _XO1008_DEVICE_H_
#define _XO1008_DEVICE_H_

#ifndef DL2106F

#include "honyar_types.h"

enum {
    POWER_OFF = 0,
    POWER_ON = 1,
}xo1008_power_state_t;

void xo1008_device_set_power(uint8_t power);

uint32_t xo1008_device_get_power_state(void);

#endif

#endif
