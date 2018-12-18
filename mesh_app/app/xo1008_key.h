#ifndef _XO1008_KEY_H_
#define _XO1008_KEY_H_

#include "honyar_types.h"

#define XO1008_KEY_LOGIC_PIN  20

enum {
    PRESS_UP = 0,
    PRESS_DOWN = 1,
}xo1008_key_state_t;

void xo1008_key_init(void);

#endif

